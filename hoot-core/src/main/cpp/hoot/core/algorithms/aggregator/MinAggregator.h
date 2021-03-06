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
#ifndef MINAGGREGATOR_H
#define MINAGGREGATOR_H

#include "ValueAggregator.h"

namespace hoot
{

using namespace std;

class MinAggregator : public ValueAggregator
{
public:

  static string className() { return "hoot::MinAggregator"; }

  MinAggregator();

  virtual double aggregate(vector<double>& d) const
  {
    double result = d[0];
    for (size_t i = 1; i < d.size(); i++)
    {
      result = min(d[i], result);
    }
    return result;
  }

  virtual QString toString() const { return "MinAggregator"; }
};

}

#endif // MINAGGREGATOR_H
