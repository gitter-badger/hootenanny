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

#ifndef DUPLICATEWAYREMOVER_H
#define DUPLICATEWAYREMOVER_H

// Hoot
#include <hoot/core/elements/Way.h>
#include <hoot/core/ops/OsmMapOperation.h>

namespace hoot
{
  class OsmMap;
  class Way;

/**
 * Searches for multiple ways that share two or more consecutive nodes if they are found, then
 * it attempts to remove the duplicate bits of the longer way. There may be some scenarios that
 * this is destructive, but in most cases it is probably useful. This routine also doesn't handle
 * the case where two ways have multiple overlapping sections. In that case only the longest
 * duplicate section will be removed.
 *
 * Area ways will be ignored.
 */
class DuplicateWayRemover : public OsmMapOperation
{
public:

  static string className() { return "hoot::DuplicateWayRemover"; }

  DuplicateWayRemover();

  void apply(shared_ptr<OsmMap>& map);

  /**
   * Remove parts of ways that are duplicates.
   */
  static void removeDuplicates(shared_ptr<OsmMap> map);

protected:

  shared_ptr<OsmMap> _map;

  bool _isCandidateWay(const ConstWayPtr& w) const;

  void _removeDuplicateNodes(shared_ptr<Way> w1, shared_ptr<Way> w2);

  void _removeNodes(shared_ptr<const Way> w, int start, int length);

  void _replaceMultiple(const shared_ptr<const Way>& oldWay,
    const shared_ptr<Way>& newWay1, const shared_ptr<Way>& newWay2);
};

}

#endif // DUPLICATEWAYREMOVER_H
