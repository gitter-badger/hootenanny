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

// Boost
#include <boost/shared_ptr.hpp>

// Hoot
#include <hoot/core/Factory.h>
#include <hoot/core/cmd/BaseCommand.h>
#include <hoot/core/io/ScriptTranslator.h>
#include <hoot/core/io/ScriptTranslatorFactory.h>
#include <hoot/core/util/ConfigOptions.h>


namespace hoot
{

class PrintOsmDocsCmd : public BaseCommand
{
public:

  static string className() { return "hoot::PrintOsmDocsCmd"; }

  PrintOsmDocsCmd() {}

  virtual QString getHelp() const
  {
    // 80 columns
    //  | <---                                                                      ---> |
    return getName() + " [documentation script]\n"
        "  Print OSM+ tag documentation.\n"
        "  If [documentation script] is not given, the default translation script\n"
        "  $HOOT_HOME/translations/PrintOsmDocs.js will be used.\n"
        "  * documentation script - The script that will print the documentation.\n";

  }

  virtual QString getName() const { return "print-tags"; }

  virtual int runSimple(QStringList /*args*/)
  {

    auto_ptr<ScriptTranslator> translator;

    // Great bit of code taken from TranslatedTagDifferencer.cpp
    boost::shared_ptr<ScriptTranslator> uut(ScriptTranslatorFactory::getInstance().createTranslator(ConfigOptions().getTagPrintingScript()));

    if (!uut)
    {
      throw HootException("Unable to find a valid translation format for: " + ConfigOptions().getTagPrintingScript());
    }

    return 0;
  }
};

HOOT_FACTORY_REGISTER(Command, PrintOsmDocsCmd)

}
