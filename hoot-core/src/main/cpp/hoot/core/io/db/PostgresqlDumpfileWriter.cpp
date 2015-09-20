/**
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

#include "PostgresqlDumpfileWriter.h"

#include <boost/shared_ptr.hpp>

#include <hoot/core/util/Settings.h>
#include <hoot/core/Factory.h>
#include <hoot/core/util/Settings.h>
#include <hoot/core/elements/Node.h>
#include <hoot/core/elements/Way.h>
#include <hoot/core/elements/Relation.h>

namespace hoot
{

HOOT_FACTORY_REGISTER(OsmMapWriter, PostgresqlDumpfileWriter)

PostgresqlDumpfileWriter::PostgresqlDumpfileWriter()
{
  setConfiguration(conf());
}

bool PostgresqlDumpfileWriter::isSupported(QString url)
{
  return false;
}

void PostgresqlDumpfileWriter::open(QString url)
{
  ;
}

void PostgresqlDumpfileWriter::close()
{
  ;
}

void PostgresqlDumpfileWriter::finalizePartial()
{
  ;
}

void PostgresqlDumpfileWriter::writePartial(const boost::shared_ptr<const hoot::Node>& n)
{
  ;
}

void PostgresqlDumpfileWriter::writePartial(const boost::shared_ptr<const hoot::Way>& w)
{
  ;
}

void PostgresqlDumpfileWriter::writePartial(const boost::shared_ptr<const hoot::Relation>& r)
{
  ;
}

void PostgresqlDumpfileWriter::setConfiguration(const hoot::Settings &conf)
{
  ;
}



}