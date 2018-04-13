/** Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Roland Olbricht et al.
 *
 * This file is part of Overpass_API.
 *
 * Overpass_API is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Overpass_API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Overpass_API.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DE__OSM3S___OVERPASS_API__STATEMENTS__COMPARE_H
#define DE__OSM3S___OVERPASS_API__STATEMENTS__COMPARE_H


#include "statement.h"


class Set_Comparison;


/* === The statement <em>compare</em> ===

''since v0.7.55''

The statement <em>compare</em> computes the diff of the data of two timestamps.
That diff can consist of any elements as well as only those with specific properties.

The statement can have a block of substatements.
The block of substatements is executed after computing the diff in the second run,
once for the old timestamp and then again for the new timestamp.
This allows to do extra computations based on the diff results.

The statement can only be used in diff mode.
In other modes its behaviour is undefined,
and in future versions it might be a syntax error to have it elsewhere.

In the first run of a diff query it returns an empty set.
In the second run of a diff query it returns the difference of the elements.
If the statement gets an evaluator as argument
then only those elements that have different values on both timestamps are returned.
If the element does not exist on one of the timstamps
then its value is regarded as the empty string.
Currently, the only purpose of such a difference is to feed it into an output statement.

The base syntax is

  compare();

In addition, an input and/or output set can be specified:

  .<Set> compare()->.<Set>;

With the evaluator, the syntax becomes

  compare(delta:<Evaluator>);

resp.

  .<Set> compare->.<Set>(delta:<Evaluator>);

In all syntax variants a block of substatements can be attached:

  compare()
  {
    <List of Substatements>
  };

resp.

  .<Set> compare(delta:<Evaluator>)->.<Set>;
  {
    <List of Substatements>
  };

*/

class Compare_Statement : public Output_Statement
{
public:
  Compare_Statement(int line_number_, const std::map< std::string, std::string >& attributes,
                     Parsed_Query& global_settings);
  virtual ~Compare_Statement();
  virtual std::string get_name() const { return "compare"; }
  virtual void add_statement(Statement* statement, std::string text);
  virtual void execute(Resource_Manager& rman);

  static Generic_Statement_Maker< Compare_Statement > statement_maker;

  virtual std::string dump_xml(const std::string& indent) const
  {
    std::string result = indent + "<compare"
      + (input != "_" ? std::string(" from=\"") + input + "\"" : "")
      + dump_xml_result_name();
    if (criterion || !substatements.empty())
      result += ">\n";

    if (criterion)
      result += criterion->dump_xml(indent + "  ");

    for (std::vector< Statement* >::const_iterator it = substatements.begin(); it != substatements.end(); ++it)
      result += *it ? (*it)->dump_xml(indent + "  ") : "";

    if (criterion || !substatements.empty())
      return result + indent + "</criterion>\n";
    return result + "/>\n";
  }

  virtual std::string dump_compact_ql(const std::string& indent) const
  {
    std::string result = (input != "_" ? std::string(".") + input + " " : "")
        + "compare" + dump_ql_result_name()
        + (criterion ? "(delta:" + criterion->dump_compact_ql(indent) + ")" : "()");

    if (!substatements.empty())
    {
      result += "{";
      for (std::vector< Statement* >::const_iterator it = substatements.begin(); it != substatements.end(); ++it)
        result += (*it)->dump_compact_ql(indent) + ";";
      result += "}";
    }
    else
      result += ";";

    return result;
  }

  virtual std::string dump_pretty_ql(const std::string& indent) const
  {
    std::string result = (input != "_" ? std::string(".") + input + " " : "")
        + "compare" + dump_ql_result_name()
        + (criterion ? "(delta:" + criterion->dump_compact_ql(indent) + ")" : "()");

    if (!substatements.empty())
    {
      result += indent + "{";
      for (std::vector< Statement* >::const_iterator it = substatements.begin(); it != substatements.end(); ++it)
        result += "\n" + (*it)->dump_pretty_ql(indent + "  ") + ";";
      result += "\n" + indent + "}";
    }
    else
      result += ";";

    return result;
  }

private:
  Evaluator* criterion;
  std::vector< Statement* > substatements;
  std::string input;
  Set_Comparison* set_comparison;
};


#endif
