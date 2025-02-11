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


#include "parameterset.h"
#include "base/parameter.h"
#include "base/latextools.h"
#include "base/tools.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "boost/foreach.hpp"

#include <fstream>
#include <numeric>

using namespace std;
using namespace rapidxml;

namespace insight
{



ParameterSet::ParameterSet()
{
}

ParameterSet::ParameterSet(const ParameterSet& o)
//: boost::ptr_map<std::string, Parameter>(o.clone())
: std::map<std::string, std::unique_ptr<Parameter> >()
{
  operator=(o);
}

ParameterSet::ParameterSet(const EntryList& entries)
{
  extend(entries);
}

ParameterSet::~ParameterSet()
{
}

void ParameterSet::operator=(const ParameterSet& o)
{
  clear();
  std::transform(o.begin(), o.end(), std::inserter(*this, end()),
                 [](const value_type& op)
                  {
                    return value_type(op.first, std::unique_ptr<Parameter>(op.second->clone()));
                  }
  );
}

ParameterSet::EntryList ParameterSet::entries() const
{
  EntryList alle;
  for ( const value_type& e: *this)
  {
    alle.push_back(SingleEntry(e.first, e.second->clone()));
  }
  return alle;
}

void ParameterSet::extend(const EntryList& entries)
{
  for ( const ParameterSet::SingleEntry& i: entries )
  {
    std::string key(boost::get<0>(i));
    SubsetParameter *p = dynamic_cast<SubsetParameter*>( boost::get<1>(i) );
    if (p && this->contains(key))
    {
      // if current parameter to insert is a subset and is already existing in current set...
        SubsetParameter *myp = dynamic_cast<SubsetParameter*>( this->find(key)->second.get() );
	myp->extend(p->entries());
	delete p;
    }
    else 
    {
      // otherwise, append, if key is not existing
      // note: insert does not replace! insertion will be omitted, if key exists already
      insert( value_type(key, std::unique_ptr<Parameter>(boost::get<1>(i))) ); // take ownership of objects in given list!
    }
  }
}

ParameterSet& ParameterSet::merge(const ParameterSet& p)
{
  EntryList entries=p.entries();
  for ( const ParameterSet::SingleEntry& i: entries )
  {
    std::string key(boost::get<0>(i));
    SubsetParameter *p = dynamic_cast<SubsetParameter*>( boost::get<1>(i) );
    if (this->contains(key))
    {
      if (p)
      {
        // merging subdict
        SubsetParameter *myp = dynamic_cast<SubsetParameter*>( this->find(key)->second.get() );
	myp->merge(*p);
	delete p;
      }
      else
      {
        // replacing
	replace(key, boost::get<1>(i)); // take ownership of objects in given list!
      }
    }
    else 
    {
      // inserting
      insert( value_type(key, std::unique_ptr<Parameter>(boost::get<1>(i))) ); // take ownership of objects in given list!
    }
  }

  return *this;
}

ParameterSet& ParameterSet::getSubset(const std::string& name) 
{ 
  if (name==".")
    return *this;
  else
  {
    try
    {
      return this->get<SubsetParameter>(name)();
    }
    catch (...)
    {
      return this->get<SelectableSubsetParameter>(name)();
    }
  }
}

const ParameterSet& ParameterSet::getSubset(const std::string& name) const
{
  if (name==".")
    return *this;
  else
  {
    try
    {
      return this->get<SubsetParameter>(name)();
    }
    catch (...)
    {
      return this->get<SelectableSubsetParameter>(name)();
    }
  }
}

std::string ParameterSet::latexRepresentation() const
{
  std::string result="";
  if (size()>0)
  {
    result= 
    "\\begin{enumerate}\n";
    for(const_iterator i=begin(); i!=end(); i++)
    {
      auto ldesc=i->second->description().toLaTeX();

      result+="\\item ";
      if (!ldesc.empty())
        result+=ldesc+"\\\\";
      result+=
          "\n"
          "\\textbf{"+SimpleLatex(i->first).toLaTeX()+"} = "
          +i->second->latexRepresentation()
          +"\n";
    }
    result+="\\end{enumerate}\n";
  }
  return result;
}

std::string ParameterSet::plainTextRepresentation(int indent) const
{
    std::string result="";
    if (size()>0)
    {
      for(const_iterator i=begin(); i!=end(); i++)
      {
        result+=std::string(indent, ' ') + (i->first) + " = " + i->second->plainTextRepresentation(indent) + '\n';
      }
    }
    return result;
}


ParameterSet* ParameterSet::cloneParameterSet() const
{
  ParameterSet *np=new ParameterSet;
  for (ParameterSet::const_iterator i=begin(); i!=end(); i++)
  {
    std::string key(i->first);
    np->insert( value_type(key, std::unique_ptr<Parameter>(i->second->clone())) );
  }
  return np;
}

void ParameterSet::appendToNode(rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node, 
    boost::filesystem::path inputfilepath) const
{
  for( const_iterator i=begin(); i!= end(); i++)
  {
    i->second->appendToNode(i->first, doc, node, inputfilepath);
  }
}

void ParameterSet::readFromNode(
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>& node,
    boost::filesystem::path inputfilepath)
{
  for( iterator i=begin(); i!= end(); i++)
  {
    i->second->readFromNode(i->first, doc, node, inputfilepath);
  }
}


void ParameterSet::packExternalFiles()
{
  for (auto& p: *this)
  {
    p.second->pack();
  }
}

void ParameterSet::removePackedData()
{
  for (auto& p: *this)
  {
    p.second->clearPackedData();
  }
}


void ParameterSet::saveToStream(std::ostream& os, const boost::filesystem::path& parent_path, std::string analysisName ) const
{
//   std::cout<<"Writing parameterset to file "<<file<<std::endl;


  // prepare XML document
  xml_document<> doc;
  xml_node<>* decl = doc.allocate_node(node_declaration);
  decl->append_attribute(doc.allocate_attribute("version", "1.0"));
  decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
  doc.append_node(decl);
  xml_node<> *rootnode = doc.allocate_node(node_element, "root");
  doc.append_node(rootnode);

  // insert analysis name
  if (analysisName != "")
  {
    xml_node<> *analysisnamenode = doc.allocate_node(node_element, "analysis");
    rootnode->append_node(analysisnamenode);
    analysisnamenode->append_attribute(doc.allocate_attribute
    (
      "name",
      doc.allocate_string(analysisName.c_str())
    ));
  }

  // store parameters
  appendToNode(doc, *rootnode, parent_path);

  os << doc;
}

void ParameterSet::saveToFile(const boost::filesystem::path& file, std::string analysisName ) const
{
    std::ofstream f(file.c_str());
    saveToStream( f, file.parent_path(), analysisName );
    f << std::endl;
    f << std::flush;
    f.close();
}

std::string ParameterSet::readFromFile(const boost::filesystem::path& file)
{
  std::string contents;
  readFileIntoString(file, contents);

  xml_document<> doc;
  doc.parse<0>(&contents[0]);
  
  xml_node<> *rootnode = doc.first_node("root");
  
  std::string analysisName;
  xml_node<> *analysisnamenode = rootnode->first_node("analysis");
  if (analysisnamenode)
  {
    analysisName = analysisnamenode->first_attribute("name")->value();
  }
  
  readFromNode(doc, *rootnode, file.parent_path());
  
  return analysisName;
}


std::ostream& operator<<(std::ostream& os, const ParameterSet& ps)
{
    os << ps.plainTextRepresentation(0);
    return os;
}


defineType(SubsetParameter);
addToFactoryTable(Parameter, SubsetParameter);


SubsetParameter::SubsetParameter()
{
}

SubsetParameter::SubsetParameter(const std::string& description,  bool isHidden, bool isExpert, bool isNecessary, int order)
: Parameter(description, isHidden, isExpert, isNecessary, order)
{
}

SubsetParameter::SubsetParameter(const ParameterSet& defaultValue, const std::string& description,  bool isHidden, bool isExpert, bool isNecessary, int order)
: Parameter(description, isHidden, isExpert, isNecessary, order),
  ParameterSet(defaultValue.entries())
{
}

// void SubsetParameter::merge(const SubsetParameter& other)
// {
//   this->merge(other);
// }

std::string SubsetParameter::latexRepresentation() const
{
  return ParameterSet::latexRepresentation();
}

std::string SubsetParameter::plainTextRepresentation(int indent) const
{
  return "\n" + ParameterSet::plainTextRepresentation(indent+1);
}

bool SubsetParameter::isPacked() const
{
  bool is_packed=false;
  for(auto& p: *this)
  {
    is_packed |= p.second->isPacked();
  }
  return is_packed;
}

void SubsetParameter::pack()
{
  for(auto& p: *this)
  {
    p.second->pack();
  }
}

void SubsetParameter::unpack(const boost::filesystem::path& basePath)
{
  for(auto& p: *this)
  {
    p.second->unpack(basePath);
  }
}

void SubsetParameter::clearPackedData()
{
  for(auto& p: *this)
  {
    p.second->clearPackedData();
  }
}



rapidxml::xml_node<>* SubsetParameter::appendToNode(const std::string& name, rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node, 
    boost::filesystem::path inputfilepath) const
{
//   std::cout<<"appending subset "<<name<<std::endl;
  using namespace rapidxml;
  xml_node<>* child = Parameter::appendToNode(name, doc, node, inputfilepath);
  ParameterSet::appendToNode(doc, *child, inputfilepath);
  return child;
}

void SubsetParameter::readFromNode(
    const std::string& name,
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>& node,
    boost::filesystem::path inputfilepath)
{
  using namespace rapidxml;
  xml_node<>* child = findNode(node, name, type());
  if (child)
  {
    ParameterSet::readFromNode(doc, *child, inputfilepath);
  }
  else
  {
    insight::Warning(
          boost::str(
            boost::format(
             "No xml node found with type '%s' and name '%s', default value '%s' is used."
             ) % type() % name % plainTextRepresentation()
           )
        );
  }
}



Parameter* SubsetParameter::clone() const
{
  return new SubsetParameter(*this, description_.simpleLatex(), isHidden_, isExpert_, isNecessary_, order_);
}



defineType(SelectableSubsetParameter);
addToFactoryTable(Parameter, SelectableSubsetParameter);

SelectableSubsetParameter::SelectableSubsetParameter(const std::string& description,  bool isHidden, bool isExpert, bool isNecessary, int order)
: Parameter(description, isHidden, isExpert, isNecessary, order)
{
}

SelectableSubsetParameter::SelectableSubsetParameter(const key_type& defaultSelection, const SubsetList& defaultValue, const std::string& description,
                                                     bool isHidden, bool isExpert, bool isNecessary, int order)
: Parameter(description, isHidden, isExpert, isNecessary, order),
  selection_(defaultSelection)
{
  for ( const SelectableSubsetParameter::SingleSubset& i: defaultValue )
  {
    std::string key(boost::get<0>(i));
    value_.insert( ItemList::value_type(key, std::unique_ptr<ParameterSet>(boost::get<1>(i))) ); // take ownership of objects in given list!
  }
}

void SelectableSubsetParameter::addItem(key_type key, const ParameterSet& ps)
{ 
    value_.insert( ItemList::value_type(key, std::unique_ptr<ParameterSet>(ps.cloneParameterSet())) );
}

void SelectableSubsetParameter::setSelection(const key_type& key, const ParameterSet& ps)
{
    selection()=key;
    operator()().merge(ps);
}

std::string SelectableSubsetParameter::latexRepresentation() const
{
//  return "(Not implemented)";
  std::ostringstream os;
  os<<"selected as ``"<<SimpleLatex(selection_).toLaTeX()<<"''\\\\"<<endl;
  os<<operator()().latexRepresentation();
  return os.str();
}

std::string SelectableSubsetParameter::plainTextRepresentation(int indent) const
{
//  return "(Not implemented)";
  std::ostringstream os;
  os<<"selected as \""<<SimpleLatex(selection_).toPlainText()<<"\"";
  if (operator()().size()>0)
  {
      os<<": \n";
      os<<operator()().plainTextRepresentation(indent+1);
  }
  return os.str();
}

bool SelectableSubsetParameter::isPacked() const
{
  bool is_packed=false;
  auto& v = this->operator()(); // get active subset
  for (auto& p: v)
  {
    is_packed |= p.second->isPacked();
  }
  return is_packed;
}

void SelectableSubsetParameter::pack()
{
  auto& v = this->operator()(); // get active subset
  for (auto& p: v)
  {
    p.second->pack();
  }
}

void SelectableSubsetParameter::unpack(const boost::filesystem::path& basePath)
{
  auto& v = this->operator()(); // get active subset
  for (auto& p: v)
  {
    p.second->unpack(basePath);
  }
}

void SelectableSubsetParameter::clearPackedData()
{
  auto& v = this->operator()(); // get active subset
  for (auto& p: v)
  {
    p.second->clearPackedData();
  }
}

rapidxml::xml_node<>* SelectableSubsetParameter::appendToNode(const std::string& name, rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node, 
    boost::filesystem::path inputfilepath) const
{
  using namespace rapidxml;
  
  xml_node<>* child = Parameter::appendToNode(name, doc, node, inputfilepath);
  child->append_attribute(doc.allocate_attribute
  (
    "value", 
    doc.allocate_string(selection_.c_str())
  ));
  
  operator()().appendToNode(doc, *child, inputfilepath);

  return child;  
}

void SelectableSubsetParameter::readFromNode
(
    const std::string& name,
    rapidxml::xml_document<>& doc,
    rapidxml::xml_node<>& node,
    boost::filesystem::path inputfilepath)
{
  using namespace rapidxml;
  xml_node<>* child = findNode(node, name, type());
  if (child)
  {
    auto valuenode=child->first_attribute("value");
    insight::assertion(valuenode, "No value attribute present!");
    selection_=valuenode->value();
    
    if (value_.find(selection_)==value_.end())
      throw insight::Exception("Invalid selection key during read of selectableSubset "+name);
    
    operator()().readFromNode(doc, *child, inputfilepath);
  }
  else
  {
    insight::Warning(
          boost::str(
            boost::format(
             "No xml node found with type '%s' and name '%s', default value '%s' is used."
             ) % type() % name % plainTextRepresentation()
           )
        );
  }
}


Parameter* SelectableSubsetParameter::clone () const
{
  SelectableSubsetParameter *np=new SelectableSubsetParameter(description_.simpleLatex(), isHidden_, isExpert_, isNecessary_, order_);
  np->selection_=selection_;
  for (ItemList::const_iterator i=value_.begin(); i!=value_.end(); i++)
  {
    std::string key(i->first);
    np->value_.insert( ItemList::value_type(key, std::unique_ptr<ParameterSet>(i->second->cloneParameterSet())) );
  }
  return np;
}



void SelectableSubsetParameter::reset(const Parameter& p)
{
  if (const auto* op = dynamic_cast<const SelectableSubsetParameter*>(&p))
  {
    Parameter::reset(p);
    selection_= op->selection_;
    for (const auto& v: op->value_)
    {
      std::string key(v.first);
      value_.insert( ItemList::value_type(key, std::unique_ptr<ParameterSet>(v.second->cloneParameterSet())) );
    }
  }
  else
    throw insight::Exception("Tried to set a "+type()+" from a different type ("+p.type()+")!");
}

ParameterSet_Validator::~ParameterSet_Validator()
{}

void ParameterSet_Validator::update(const ParameterSet& ps)
{
    errors_.clear();
    warnings_.clear();
    ps_=ps;
}

bool ParameterSet_Validator::isValid() const
{
    return (warnings_.size()==0) && (errors_.size()==0);
}

const ParameterSet_Validator::WarningList& ParameterSet_Validator::ParameterSet_Validator::warnings() const
{
    return warnings_;
}

const ParameterSet_Validator::ErrorList& ParameterSet_Validator::ParameterSet_Validator::errors() const
{
    return errors_;
}



ParameterSet_Visualizer::ParameterSet_Visualizer()
  : defaultProgressDisplayer_(),
    progress_(&defaultProgressDisplayer_)
{}

ParameterSet_Visualizer::~ParameterSet_Visualizer()
{}

void ParameterSet_Visualizer::update(const ParameterSet& ps)
{
    ps_=ps;
}




void ParameterSet_Visualizer::setIcon(QIcon*)
{
}

void ParameterSet_Visualizer::setProgressDisplayer(ProgressDisplayer* pd)
{
  if (pd!=nullptr)
    progress_=pd;
  else
    progress_=&defaultProgressDisplayer_;
}

}

