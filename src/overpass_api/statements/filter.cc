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

#include "filter.h"
#include "tag_value.h"
#include "../data/tag_store.h"

#include <algorithm>
#include <sstream>


class Filter_Constraint : public Query_Constraint
{
  public:
    bool delivers_data(Resource_Manager& rman) { return false; }

    Filter_Constraint(Filter_Statement& stmt_) : stmt(&stmt_) {}
    bool get_ranges(Resource_Manager& rman, std::set< std::pair< Uint32_Index, Uint32_Index > >& ranges) { return false; }
    bool get_ranges(Resource_Manager& rman, std::set< std::pair< Uint31_Index, Uint31_Index > >& ranges) { return false; }
    void filter(Resource_Manager& rman, Set& into) {}
    void filter(const Statement& query, Resource_Manager& rman, Set& into);
    virtual ~Filter_Constraint() {}

  private:
    Filter_Statement* stmt;
};


template< typename Index, typename Maybe_Attic >
void eval_elems(std::map< Index, std::vector< Maybe_Attic > >& items,
    Set_With_Context& into_context, Eval_Task& task)
{
  for (typename std::map< Index, std::vector< Maybe_Attic > >::iterator it_idx = items.begin();
      it_idx != items.end(); ++it_idx)
  {
    std::vector< Maybe_Attic > local_into;
    local_into.reserve(it_idx->second.size());

    for (typename std::vector< Maybe_Attic >::const_iterator it_elem = it_idx->second.begin();
        it_elem != it_idx->second.end(); ++it_elem)
    {
      if (string_represents_boolean_true(
          task.eval(into_context.get_context(it_idx->first, *it_elem), 0)))
        local_into.push_back(*it_elem);
    }

    local_into.swap(it_idx->second);
  }
}


void Filter_Constraint::filter(const Statement& query, Resource_Manager& rman, Set& into)
{
  if (!stmt || !stmt->get_criterion())
    return;

  Requested_Context requested_context = stmt->get_criterion()->request_context();
  Prepare_Task_Context context(requested_context, query, rman);

  Owner< Eval_Task > task(stmt->get_criterion()->get_string_task(context, 0));

  Set_With_Context into_context;
  into_context.name = "";
  into_context.parent = &context;
  into_context.prefetch(requested_context.object_usage, into, query, rman);

  if (task)
  {
    eval_elems(into.nodes, into_context, *task);
    eval_elems(into.attic_nodes, into_context, *task);
    eval_elems(into.ways, into_context, *task);
    eval_elems(into.attic_ways, into_context, *task);
    eval_elems(into.relations, into_context, *task);
    eval_elems(into.attic_relations, into_context, *task);
    eval_elems(into.areas, into_context, *task);
    eval_elems(into.deriveds, into_context, *task);
  }
}


//-----------------------------------------------------------------------------


Filter_Statement::Statement_Maker Filter_Statement::statement_maker;
Filter_Statement::Criterion_Maker Filter_Statement::criterion_maker;


Statement* Filter_Statement::Criterion_Maker::create_criterion(const Token_Node_Ptr& tree_it,
    const std::string& type, const std::string& into,
    Statement::Factory& stmt_factory, Parsed_Query& global_settings, Error_Output* error_output)
{
  Statement* filter = 0;
  uint line_nr = tree_it->line_col.first;

  if (tree_it->token == ":" && tree_it->rhs)
  {
    Statement* criterion = stmt_factory.create_evaluator(
        tree_it.rhs(), Statement::elem_eval_possible, Statement::Single_Return_Type_Checker(Statement::string));
    if (criterion)
    {
      std::map< std::string, std::string > attributes;
      filter = new Filter_Statement(line_nr, attributes, global_settings);
      if (filter)
        filter->add_statement(criterion, "");
    }
  }

  return filter;
}


Filter_Statement::Filter_Statement
    (int line_number_, const std::map< std::string, std::string >& input_attributes, Parsed_Query& global_settings)
    : Output_Statement(line_number_), criterion(0)
{
  std::map< std::string, std::string > attributes;

  eval_attributes_array(get_name(), attributes, input_attributes);
}


Filter_Statement::~Filter_Statement()
{
  for (std::vector< Query_Constraint* >::const_iterator it = constraints.begin();
      it != constraints.end(); ++it)
    delete *it;
}


void Filter_Statement::add_statement(Statement* statement, std::string text)
{
  Evaluator* tag_value = dynamic_cast< Evaluator* >(statement);
  if (tag_value)
  {
    if (!criterion)
      criterion = tag_value;
    else
      add_static_error("A filter statement can have at most one evaluator statement.");
  }
  else
    substatement_error(get_name(), statement);
}


void Filter_Statement::execute(Resource_Manager& rman)
{
  Set into;

  Filter_Constraint constraint(*this);
  constraint.filter(rman, into);

  transfer_output(rman, into);
  rman.health_check(*this);
}


Query_Constraint* Filter_Statement::get_query_constraint()
{
  constraints.push_back(new Filter_Constraint(*this));
  return constraints.back();
}
