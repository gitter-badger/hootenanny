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
#include "OsmMapReaderFactory.h"

// hoot
#include <hoot/core/Factory.h>
#include <hoot/core/io/OsmMapReader.h>
#include <hoot/core/io/PartialOsmMapReader.h>
#include <hoot/core/io/ElementInputStream.h>
#include <hoot/core/util/ConfigOptions.h>

namespace hoot
{

OsmMapReaderFactory OsmMapReaderFactory::_theInstance;

OsmMapReaderFactory::OsmMapReaderFactory()
{
}

bool OsmMapReaderFactory::hasReader(QString url)
{
  vector<std::string> names =
    Factory::getInstance().getObjectNamesByBase(OsmMapReader::className());
  bool result = false;
  for (size_t i = 0; i < names.size() && !result; ++i)
  {
    boost::shared_ptr<OsmMapReader> reader(Factory::getInstance().constructObject<OsmMapReader>(names[i]));
    if (reader->isSupported(url))
    {
      result = true;
    }
  }
  return result;
}

bool OsmMapReaderFactory::hasElementInputStream(QString url)
{
  bool result = false;
  boost::shared_ptr<OsmMapReader> reader = createReader(url, true, Status::Unknown1);
  boost::shared_ptr<ElementInputStream> eis = dynamic_pointer_cast<ElementInputStream>(reader);
  if (eis)
  {
    LOG_DEBUG("Streaming: reader support!");
    result = true;
  }

  return result;
}

bool OsmMapReaderFactory::hasPartialReader(QString url)
{
  bool result = false;
  boost::shared_ptr<OsmMapReader> reader = createReader(url, true, Status::Unknown1);
  boost::shared_ptr<PartialOsmMapReader> pr = dynamic_pointer_cast<PartialOsmMapReader>(reader);
  if (pr)
  {
    result = true;
  }

  return result;
}

boost::shared_ptr<OsmMapReader> OsmMapReaderFactory::createReader(QString url, bool useDataSourceIds,
                                                           Status defaultStatus)
{
  QString readerOverride = ConfigOptions().getOsmMapReaderFactoryReader();

  boost::shared_ptr<OsmMapReader> reader;
  if (readerOverride != "")
  {
    reader.reset(Factory::getInstance().constructObject<OsmMapReader>(readerOverride));
    LOG_DEBUG("Using reader: " << readerOverride);
  }

  vector<std::string> names =
    Factory::getInstance().getObjectNamesByBase(OsmMapReader::className());
  for (size_t i = 0; i < names.size() && !reader; ++i)
  {
    //LOG_DEBUG("Checking input " << url << " with reader " << names[i]);
    reader.reset(Factory::getInstance().constructObject<OsmMapReader>(names[i]));
    if (reader->isSupported(url))
    {
      LOG_DEBUG("Using input reader: " << names[i]);
    }
    else
    {
      reader.reset();
    }
  }

  if (!reader)
  {
    throw HootException("A valid reader could not be found for the URL: " + url);
  }

  reader->setUseDataSourceIds(useDataSourceIds);
  reader->setDefaultStatus(defaultStatus);

  return reader;
}

void OsmMapReaderFactory::read(boost::shared_ptr<OsmMap> map, QString url, bool useDataSourceIds,
                               Status defaultStatus)
{
  boost::shared_ptr<OsmMapReader> reader =
    getInstance().createReader(url, useDataSourceIds, defaultStatus);
  reader->open(url);
  reader->read(map);
}

}
