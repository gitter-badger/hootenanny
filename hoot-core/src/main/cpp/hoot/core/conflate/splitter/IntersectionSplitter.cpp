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

#include "IntersectionSplitter.h"

// Hoot
#include <hoot/core/Factory.h>
#include <hoot/core/algorithms/WaySplitter.h>
#include <hoot/core/elements/Way.h>
#include <hoot/core/schema/OsmSchema.h>

// Qt
#include <QDebug>

namespace hoot
{

HOOT_FACTORY_REGISTER(OsmMapOperation, IntersectionSplitter)

IntersectionSplitter::IntersectionSplitter()
{

}

IntersectionSplitter::IntersectionSplitter(boost::shared_ptr<OsmMap> map)
{
  _map = map;
}

void IntersectionSplitter::_mapNodesToWay(boost::shared_ptr<Way> way)
{
  long wId = way->getId();

  const std::vector<long>& nodes = way->getNodeIds();
  for (size_t i = 0; i < nodes.size(); i++)
  {
    _nodeToWays.insert(nodes[i], wId);

    if (_nodeToWays.count(nodes[i]) > 1)
    {
      _todoNodes.insert(nodes[i]);
    }
  }
}

void IntersectionSplitter::_mapNodesToWays()
{
  _nodeToWays.clear();

  const WayMap& ways = _map->getWays();
  for (WayMap::const_iterator it = ways.begin(); it != ways.end(); ++it)
  {
    boost::shared_ptr<Way> w = it->second;
    if (OsmSchema::getInstance().isLinearHighway(w->getTags(), w->getElementType()) ||
        OsmSchema::getInstance().isLinearWaterway(*w))
    {
      _mapNodesToWay(w);
    }
//    else
//    {
//      LOG_DEBUG("IntersectionSplitter skipping way: " << w->getId());
//    }
  }
}

void IntersectionSplitter::_removeWayFromMap(boost::shared_ptr<Way> way)
{
  long wId = way->getId();

  const std::vector<long>& nodes = way->getNodeIds();
  for (size_t i = 0; i < nodes.size(); i++)
  {
    _nodeToWays.remove(nodes[i], wId);
  }
}

void IntersectionSplitter::splitIntersections(boost::shared_ptr<OsmMap> map)
{
  IntersectionSplitter is(map);
  return is.splitIntersections();
}

void IntersectionSplitter::splitIntersections()
{
  LOG_INFO("Splitting intersections...");
  // make a map of nodes to ways.
  _mapNodesToWays();

  // go through all the nodes
  while (_todoNodes.isEmpty() == false)
  {
    long nodeId = *_todoNodes.begin();
    _todoNodes.remove(nodeId);

    if (Log::getInstance().isInfoEnabled() && _todoNodes.size() % 1000 == 0)
    {
      cout << "  Intersection splitter todo: " << _todoNodes.size() << "       \r";
      cout.flush();
    }

    // if the node is part of two or more ways
    if (_nodeToWays.count(nodeId) >= 2)
    {
      // evaluate each way for splitting
      QList<long> ways = _nodeToWays.values(nodeId);
      for (QList<long>::const_iterator way = ways.begin(); way != ways.end(); ++way)
      {
        _splitWay(*way, nodeId);
      }
    }
  }

  if (Log::getInstance().isInfoEnabled())
  {
    cout << endl;
  }
}

void IntersectionSplitter::_splitWay(long wayId, long nodeId)
{
  boost::shared_ptr<Way> way = _map->getWay(wayId);
  if (way == 0)
  {
    //LOG_WARN("way at " << wayId << " does not exist.");
    return;
  }
  boost::shared_ptr<const Node> node = _map->getNode(nodeId);

  //LOG_DEBUG("Splitting way: " << way->getId() << " at node: " << node->getId());

  // find the first index of the split node that isn't an endpoint.
  int firstIndex = -1;
  const std::vector<long>& nodeIds = way->getNodeIds();
  for (size_t i = 1; i < nodeIds.size() - 1; i++)
  {
    if (nodeIds[i] == nodeId)
    {
      firstIndex = i;
      break;
    }
  }

  // if the first index wasn't an endpoint.
  if (firstIndex != -1)
  {
    // split the way and remove it from the map
    WayLocation wl(_map, way, firstIndex, 0.0);
    vector< boost::shared_ptr<Way> > splits = WaySplitter::split(_map, way, wl);

    // if a split occurred.
    if (splits.size() > 1)
    {
      _removeWayFromMap(way);

      // go through all the resulting splits
      for (size_t i = 0; i < splits.size(); i++)
      {
        // add the new ways nodes just in case the way was self intersecting.
        _mapNodesToWay(splits[i]);
      }
    }
  }
}

void IntersectionSplitter::apply(boost::shared_ptr<OsmMap>& map)
{
  splitIntersections(map);
}

}
