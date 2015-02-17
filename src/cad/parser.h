/*
 * This file is part of Insight CAE, a workbench for Computer-Aided Engineering 
 * Copyright (C) 2014  Hannes Kroeger <hannes@kroegeronline.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef INSIGHT_CAD_PARSER_H
#define INSIGHT_CAD_PARSER_H

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include "solidmodel.h"
#include "datum.h"
#include "sketch.h"
#include "freeship_interface.h"

#include "base/boost_include.h"

#include "boost/spirit/include/qi.hpp"
#include "boost/variant/recursive_variant.hpp"
#include "boost/spirit/repository/include/qi_confix.hpp"
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/phoenix/function.hpp>
#include <boost/phoenix/function/adapt_callable.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/home/classic/utility/distinct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/utility/enable_if.hpp>

namespace mapkey_parser
{
  BOOST_SPIRIT_TERMINAL(mapkey);
  
//   namespace tag { struct mapkey{}; } // tag identifying placeholder
//   typedef unspecified<tag::mapkey> mapkey_type;
//   mapkey_type const mapkey = {};   // placeholder itself
}

namespace boost { namespace spirit
{
    // We want custom_parser::iter_pos to be usable as a terminal only,
    // and only for parser expressions (qi::domain).
    template <>
    struct use_terminal<qi::domain, mapkey_parser::tag::mapkey>
      : mpl::true_
    {};
}}

namespace mapkey_parser
{
    template<class T>
    struct mapkey_parser
      : boost::spirit::qi::primitive_parser<mapkey_parser<T> >
    {
        typedef std::map<std::string, T>& RefMap;
	
	const RefMap& map_;
	
	mapkey_parser(const RefMap& map)
	: map_(map)
	{}
	
        // Define the attribute type exposed by this parser component
        template <typename Context, typename Iterator>
        struct attribute
        {
            typedef std::string type;
        };
 
        // This function is called during the actual parsing process
        template <typename Iterator, typename Context, typename Skipper, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context&, Skipper const& skipper, Attribute& attr) const
        {
            boost::spirit::qi::skip_over(first, last, skipper);
	    std::string key(first, last);

	    typename std::map<std::string, T>::const_iterator it=map_.find(key);
            if (it!=map_.end())
            {
                boost::spirit::traits::assign_to(key, attr);
                return true;
            }
            return false;
        }
 
        // This function is called during error handling to create
        // a human readable string for the error context.
        template <typename Context>
        boost::spirit::info what(Context&) const
        {
            return boost::spirit::info("mapkey");
        }
    };
}


namespace insight {
namespace cad {
namespace parser {

namespace qi = boost::spirit::qi;
namespace repo = boost::spirit::repository;
namespace phx   = boost::phoenix;


typedef double scalar;
typedef arma::mat vector;
typedef Datum::Ptr datum;
typedef SolidModel::Ptr solidmodel;
typedef std::pair<std::string, solidmodel > modelstep;
typedef std::vector<modelstep> model;

typedef boost::tuple<std::string, vector, vector, boost::optional<bool> > viewdef;


double dot(const vector& v1, const vector& v2);
BOOST_PHOENIX_ADAPT_FUNCTION(vector, vec3_, vec3, 3);
BOOST_PHOENIX_ADAPT_FUNCTION(vector, cross_, cross, 2);
BOOST_PHOENIX_ADAPT_FUNCTION(vector, trans_, arma::trans, 1);
BOOST_PHOENIX_ADAPT_FUNCTION(double, dot_, dot, 2);

void writeViews(const boost::filesystem::path& file, const solidmodel& model, const std::vector<viewdef>& viewdefs);
BOOST_PHOENIX_ADAPT_FUNCTION(void, writeViews_, writeViews, 3);

FeatureSetPtr queryEdges(const SolidModel& m, const std::string& filterexpr, const FeatureSetList& of);
BOOST_PHOENIX_ADAPT_FUNCTION(FeatureSetPtr, queryEdges_, queryEdges, 3);

typedef boost::variant<scalar, vector>  ModelSymbol;
typedef std::vector<boost::fusion::vector2<std::string, ModelSymbol> > ModelSymbols;


struct Model
{
  gp_Ax3 placement_;
  
  typedef boost::shared_ptr<Model> Ptr;
  
  Model(const ModelSymbols& syms = ModelSymbols());
  
  typedef qi::symbols<char, scalar> scalarSymbolTable;
  scalarSymbolTable scalarSymbols;
  struct vectorSymbolTable : public qi::symbols<char, vector> {} vectorSymbols;
  struct datumSymbolTable : public qi::symbols<char, datum> {} datumSymbols;
  typedef qi::symbols<char, solidmodel> modelstepSymbolTable;
  modelstepSymbolTable modelstepSymbols;
//   std::map<std::string, SolidModel::Ptr> modelstepSymbols;

  struct edgeFeaturesSymbolTable : public qi::symbols<char, FeatureSetPtr> {} edgeFeatureSymbols;

  struct modelSymbolTable : public qi::symbols<char, Model::Ptr> {} modelSymbols;
};

Model::Ptr loadModel(const std::string& name, const ModelSymbols& syms);
BOOST_PHOENIX_ADAPT_FUNCTION(Model::Ptr, loadModel_, loadModel, 2);


}

bool parseISCADModelStream(std::istream& in, parser::Model::Ptr& m, int* failloc=NULL);
bool parseISCADModelFile(const boost::filesystem::path& fn, parser::Model::Ptr& m);


}
}

#undef BOOST_SPIRIT_DEBUG

#endif // INSIGHT_CAD_PARSER_H
