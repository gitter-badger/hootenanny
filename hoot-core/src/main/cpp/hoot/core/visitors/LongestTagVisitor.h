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
#ifndef LONGESTTAGVISITOR_H
#define LONGESTTAGVISITOR_H

// hoot
#include <hoot/core/OsmMap.h>
#include <hoot/core/elements/ElementVisitor.h>
#include <hoot/core/OsmMapConsumer.h>

// Qt
#include <QSet>

#include "SingleStatistic.h"

namespace hoot
{
using namespace std;

/**
 * Sums the length of all the ways. The map projection is used so to get meters the map must be
 * first reprojected into meters.
 */
class LongestTagVisitor : public ElementVisitor, public OsmMapConsumer, public SingleStatistic
{
public:

  static string className() { return "hoot::LongestTagVisitor"; }

  LongestTagVisitor() : _longestTag() {}

  virtual ~LongestTagVisitor() {}

  double getStat() const { return _longestTag; }

  QString getLongestTag() const { return _tag; }

  virtual void setOsmMap(const OsmMap* map) { _map = map; }

  virtual void visit(const ConstElementPtr& e)
  {
    const Tags& t = e->getTags();

    for (Tags::const_iterator it = t.begin(); it != t.end(); ++it)
    {
      if (it.value().size() > _longestTag)
      {
        _longestTag = it.value().size();
        _tag = it.key() + "=" + it.value();
      }
    }
  }

private:
  int _longestTag;
  QString _tag;
  const OsmMap* _map;
};

}

#endif // LONGESTTAGVISITOR_H
