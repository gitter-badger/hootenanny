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
#include "TagMergerFactory.h"

// hoot
#include <hoot/core/Factory.h>
#include <hoot/core/util/ConfigOptions.h>

namespace hoot
{

TagMergerFactory TagMergerFactory::_theInstance;

TagMergerFactory::TagMergerFactory()
{
  _default = 0;
}

TagMergerFactory::~TagMergerFactory()
{
  reset();
}

const TagMerger& TagMergerFactory::getDefault()
{
  if (_default == 0)
  {
    QString defaultName = ConfigOptions().getTagMergerDefault();
    LOG_DEBUG("Default tag merger is: " << defaultName);
    _default = &getMerger(defaultName);
  }

  return *_default;
}

const TagMerger& TagMergerFactory::getMerger(const QString& name)
{
  boost::shared_ptr<const TagMerger> result;
  QHash<QString, boost::shared_ptr<const TagMerger> >::const_iterator it = _mergers.find(name);
  if (it == _mergers.end())
  {
    result.reset(Factory::getInstance().constructObject<TagMerger>(name.toStdString()));
    _mergers.insert(name, result);
  }
  else
  {
    result = it.value();
  }

  return *result;
}

Tags TagMergerFactory::mergeTags(const Tags& t1, const Tags& t2, ElementType et)
{
  return getInstance().getDefault().mergeTags(t1, t2, et);
}

void TagMergerFactory::reset()
{
  _default = 0;
  _mergers.clear();
}

}
