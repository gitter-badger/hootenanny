/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */
#include "UnifyingConflator.h"

// hoot
#include <hoot/core/Factory.h>
#include <hoot/core/MapReprojector.h>
#include <hoot/core/conflate/Merger.h>
#include <hoot/core/conflate/MarkForReviewMergerCreator.h>
#include <hoot/core/conflate/MatchFactory.h>
#include <hoot/core/conflate/MatchThreshold.h>
#include <hoot/core/conflate/MergerFactory.h>
#include <hoot/core/conflate/match-graph/GreedyConstrainedMatches.h>
#include <hoot/core/conflate/match-graph/OptimalConstrainedMatches.h>
#include <hoot/core/conflate/polygon/BuildingMergerCreator.h>
#include <hoot/core/io/OsmMapWriterFactory.h>
#include <hoot/core/ops/NamedOp.h>
#include <hoot/core/util/ConfigOptions.h>

// standard
#include <algorithm>

// tgs
#include <tgs/System/SystemInfo.h>
#include <tgs/System/Time.h>
#include <tgs/System/Timer.h>

namespace hoot
{
using namespace Tgs;

HOOT_FACTORY_REGISTER(OsmMapOperation, UnifyingConflator)

UnifyingConflator::UnifyingConflator() :
  _matchFactory(MatchFactory::getInstance()),
  _settings(Settings::getInstance())
{
  _reset();
}

UnifyingConflator::UnifyingConflator(shared_ptr<MatchThreshold> matchThreshold) :
  _matchFactory(MatchFactory::getInstance()),
  _settings(Settings::getInstance())
{
  _matchThreshold = matchThreshold;
  _reset();
}


UnifyingConflator::~UnifyingConflator()
{
  _reset();
}

void UnifyingConflator::_addScoreTags(const ElementPtr& e, const MatchClassification& mc)
{
  Tags& tags = e->getTags();

  tags.appendValue(reviewScoreKey(), mc.getReviewP());
  tags.appendValue(matchScoreKey(), mc.getMatchP());
  tags.appendValue(missScoreKey(), mc.getMissP());
}

void UnifyingConflator::_addReviewTags(const OsmMapPtr& map, const vector<const Match*>& matches)
{
  if (ConfigOptions(_settings).getConflateAddScoreTags())
  {
    for (size_t i = 0; i < matches.size(); i++)
    {
      const Match* m = matches[i];
      const MatchClassification& mc = m->getClassification();
      set< pair<ElementId, ElementId> > pairs = m->getMatchPairs();
      for (set< pair<ElementId, ElementId> >::const_iterator it = pairs.begin();
           it != pairs.end(); ++it)
      {
        if (mc.getReviewP() > 0.0)
        {
          ElementPtr e1 = map->getElement(it->first);
          ElementPtr e2 = map->getElement(it->second);

          _addScoreTags(e1, mc);
          _addScoreTags(e2, mc);
          e1->getTags().appendValue(scoreUuidKey(), e2->getTags().getCreateUuid());
          e2->getTags().appendValue(scoreUuidKey(), e1->getTags().getCreateUuid());
        }
      }
    }
  }
}

void UnifyingConflator::apply(shared_ptr<OsmMap>& map)
{
  Timer timer;
  _reset();

  NamedOp(ConfigOptions().getUnifyPreOps().split(";", QString::SkipEmptyParts)).apply(map);

  _stats.append(SingleStat("Apply Pre Ops Time (sec)", timer.getElapsedAndRestart()));

  // will reproject if necessary.
  MapReprojector::reprojectToPlanar(map);

  _stats.append(SingleStat("Project to Planar Time (sec)", timer.getElapsedAndRestart()));

  if (Log::getInstance().isDebugEnabled())
  {
    LOG_DEBUG("Writing debug map.");
    OsmMapPtr debug(new OsmMap(map));
    MapReprojector::reprojectToWgs84(debug);
    OsmMapWriterFactory::write(debug, ConfigOptions().getDebugMapFilename());

    _stats.append(SingleStat("Write Debug Map Time (sec)", timer.getElapsedAndRestart()));
  }

  LOG_DEBUG("Creating matches...");
  // find all the matches in this map
  if (_matchThreshold.get())
  {
    //ScoreMatches logic seems to be the only one that needs to pass in the match threshold now when
    //the optimize param is activated.  Otherwise, we get the match threshold information from the
    //config.
    _matchFactory.createMatches(map, _matches, _bounds, _matchThreshold);
  }
  else
  {
    _matchFactory.createMatches(map, _matches, _bounds);
  }
  LOG_DEBUG("Match count: " << _matches.size());
  LOG_DEBUG(SystemInfo::getMemoryUsageString());

  double findMatchesTime = timer.getElapsedAndRestart();
  _stats.append(SingleStat("Find Matches Time (sec)", findMatchesTime));
  _stats.append(SingleStat("Number of Matches Found", _matches.size()));
  _stats.append(SingleStat("Number of Matches Found per Second",
    (double)_matches.size() / findMatchesTime));

  vector<const Match*> allMatches = _matches;

  // add review tags to all matches that have some review component
  _addReviewTags(map, allMatches);
  LOG_INFO("Pre-constraining match count: " << allMatches.size());

  _stats.append(SingleStat("Number of Matches Before Whole Groups", _matches.size()));

  // If there are groups of matches that should not be optimized, remove them before optimization.
  MatchSetVector matchSets;
  _removeWholeGroups(_matches, matchSets, map);
  _stats.append(SingleStat("Number of Whole Groups", matchSets.size()));

  // Globally optimize the set of matches to maximize the conflation score.
  {
    OptimalConstrainedMatches cm(map);
    vector<const Match*> cmMatches;

    if (ConfigOptions(_settings).getUnifyEnableOptimalConstrainedMatches())
    {
      cm.addMatches(_matches.begin(), _matches.end());

      cm.setTimeLimit(ConfigOptions(_settings).getUnifyOptimizerTimeLimit());

      double cmStart = Time::getTime();
//      vector<const Match*> cmMatches = cm.calculateSubset();
      cmMatches = cm.calculateSubset();
      LOG_INFO("CM took: " << Time::getTime() - cmStart << "s.");
      LOG_INFO("CM Score: " << cm.getScore());
      LOG_DEBUG(SystemInfo::getMemoryUsageString());
    }

    GreedyConstrainedMatches gm(map);
    gm.addMatches(_matches.begin(), _matches.end());
    double gmStart = Time::getTime();
    vector<const Match*> gmMatches = gm.calculateSubset();
    LOG_INFO("GM took: " << Time::getTime() - gmStart << "s.");
    LOG_INFO("GM Score: " << gm.getScore());

    if (gm.getScore() > cm.getScore())
    {
      _matches = gmMatches;
    }
    else
    {
      _matches = cmMatches;
    }
  }

  double optimizeMatchesTime = timer.getElapsedAndRestart();
  _stats.append(SingleStat("Optimize Matches Time (sec)", optimizeMatchesTime));
  _stats.append(SingleStat("Number of Optimized Matches", _matches.size()));
  _stats.append(SingleStat("Number of Matches Optimized per Second",
    (double)allMatches.size() / optimizeMatchesTime));

  LOG_DEBUG(SystemInfo::getMemoryUsageString());

//  #warning validateConflictSubset is on, this is slow.
//  _validateConflictSubset(map, _matches);

  LOG_INFO("Post constraining match count: " << _matches.size());

  {
    // search the matches for groups (subgraphs) of matches. In other words, groups where all the
    // matches are interrelated by element id
    MatchGraph mg;
    mg.addMatches(_matches.begin(), _matches.end());
    vector< set<const Match*, MatchPtrComparator> > tmpMatchSets =
      mg.findSubgraphs(map);
    matchSets.insert(matchSets.end(), tmpMatchSets.begin(), tmpMatchSets.end());
    LOG_DEBUG(SystemInfo::getMemoryUsageString());
  }

  LOG_DEBUG("Match sets count: " << matchSets.size());
  LOG_DEBUG(SystemInfo::getMemoryUsageString());
  /// @todo would it help to sort the matches so the biggest or best ones get merged first?

  // convert all the match sets into mergers.
  for (size_t i = 0; i < matchSets.size(); ++i)
  {
    _mergerFactory->createMergers(map, matchSets[i], _mergers);
  }

  LOG_DEBUG(SystemInfo::getMemoryUsageString());
  // don't need the matches any more
  _deleteAll(allMatches);
  _matches.clear();

  LOG_DEBUG(SystemInfo::getMemoryUsageString());
  _mapElementIdsToMergers();
  LOG_DEBUG(SystemInfo::getMemoryUsageString());

  _stats.append(SingleStat("Create Mergers Time (sec)", timer.getElapsedAndRestart()));

  vector< pair<ElementId, ElementId> > replaced;
  for (size_t i = 0; i < _mergers.size(); ++i)
  {
    _mergers[i]->apply(map, replaced);

    // update any mergers that reference the replaced values
    _replaceElementIds(replaced);
    replaced.clear();
    if (Log::getInstance().getLevel() == Log::Debug)
    {
      cout << "Applying mergers: " << i + 1 << " / " << _mergers.size() << "       \r" << flush;
    }
  }
  if (Log::getInstance().getLevel() == Log::Debug)
  {
    cout << endl;
  }
  LOG_DEBUG(SystemInfo::getMemoryUsageString());
  size_t mergerCount = _mergers.size();
  // free up any used resources.
  _reset();
  LOG_DEBUG(SystemInfo::getMemoryUsageString());

  double mergersTime = timer.getElapsedAndRestart();
  _stats.append(SingleStat("Apply Mergers Time (sec)", mergersTime));
  _stats.append(SingleStat("Mergers Applied per Second", (double)mergerCount / mergersTime));

  NamedOp(ConfigOptions().getUnifyPostOps().split(";", QString::SkipEmptyParts)).apply(map);

  _stats.append(SingleStat("Apply Post Ops Time (sec)", timer.getElapsedAndRestart()));
}

void UnifyingConflator::_mapElementIdsToMergers()
{
  _e2m.clear();

  for (size_t i = 0; i < _mergers.size(); ++i)
  {
    set<ElementId> impacted = _mergers[i]->getImpactedElementIds();

    for (set<ElementId>::const_iterator it = impacted.begin(); it != impacted.end(); ++it)
    {
      _e2m[*it].push_back(_mergers[i]);
    }
  }
}

void UnifyingConflator::_removeWholeGroups(vector<const Match*>& matches,
  MatchSetVector &matchSets, const OsmMapPtr &map)
{
  // search the matches for groups (subgraphs) of matches. In other words, groups where all the
  // matches are interrelated by element id
  MatchGraph mg;
  mg.setCheckForConflicts(false);
  mg.addMatches(_matches.begin(), _matches.end());
  MatchSetVector tmpMatchSets = mg.findSubgraphs(map);

  matchSets.reserve(matchSets.size() + tmpMatchSets.size());
  vector<const Match*> leftovers;

  for (size_t i = 0; i < tmpMatchSets.size(); i++)
  {
    bool wholeGroup = false;
    for (MatchSet::const_iterator it = tmpMatchSets[i].begin();
         it != tmpMatchSets[i].end(); ++it)
    {
      if ((*it)->isWholeGroup())
      {
        wholeGroup = true;
      }
    }

    if (wholeGroup)
    {
      matchSets.push_back(tmpMatchSets[i]);
    }
    else
    {
      leftovers.insert(leftovers.end(), tmpMatchSets[i].begin(), tmpMatchSets[i].end());
    }
  }

  matches = leftovers;
}

void UnifyingConflator::_replaceElementIds(const vector< pair<ElementId, ElementId> >& replaced)
{
  for (size_t i = 0; i < replaced.size(); ++i)
  {
    HashMap<ElementId, vector<Merger*> >::const_iterator it = _e2m.find(replaced[i].first);
    if (it != _e2m.end())
    {
      const vector<Merger*>& mergers = it->second;
      // replace the element id in all mergers.
      for (size_t i = 0; i < mergers.size(); ++i)
      {
        mergers[i]->replace(replaced[i].first, replaced[i].second);
        _e2m[replaced[i].second].push_back(mergers[i]);
      }
      // don't need to hold on to the old reference any more.
      _e2m.erase(it->first);
    }
  }
}

void UnifyingConflator::setConfiguration(const Settings &conf)
{
  _settings = conf;

  _matchThreshold.reset();
  _mergerFactory.reset();
  _reset();
}

void UnifyingConflator::_reset()
{
  if (_mergerFactory == 0)
  {
    _mergerFactory.reset(new MergerFactory());
    // register the mark for review merger first so all reviews get tagged before another merger
    // gets a chance.
    _mergerFactory->registerCreator(new MarkForReviewMergerCreator());
    _mergerFactory->registerDefaultCreators();
  }

  _e2m.clear();
  _deleteAll(_matches);
  _deleteAll(_mergers);
}

void UnifyingConflator::_validateConflictSubset(const ConstOsmMapPtr& map,
                                                vector<const Match*> matches)
{
  for (size_t i = 0; i < matches.size(); i++)
  {
    for (size_t j = 0; j < matches.size(); j++)
    {
      if (i < j && MergerFactory::getInstance().isConflicting(map, matches[i], matches[j]))
      {
        LOG_DEBUG("Conflict");
        LOG_DEBUG(matches[i]->toString());
        LOG_DEBUG(matches[j]->toString());
      }
    }
  }
}

}
