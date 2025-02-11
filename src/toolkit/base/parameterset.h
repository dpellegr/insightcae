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


#ifndef INSIGHT_PARAMETERSET_H
#define INSIGHT_PARAMETERSET_H

#include "base/exception.h"
#include "base/parameters.h"
#include "base/progressdisplayer.h"
#include "base/progressdisplayer/textprogressdisplayer.h"

//#include "boost/ptr_container/ptr_map.hpp"
//#include "boost/shared_ptr.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/fusion/tuple.hpp"
#include "boost/algorithm/string.hpp"

#include "rapidxml/rapidxml.hpp"

#include <memory>
#include <map>
#include <vector>
#include <iostream>

class QoccViewWidget;
class QModelTree;
class QIcon;


namespace insight {
  



class SubsetParameter;
class SelectableSubsetParameter;




class ParameterSet
//  : public boost::ptr_map<std::string, Parameter>
  : public std::map<std::string, std::unique_ptr<Parameter> >
{

public:
  typedef std::shared_ptr<ParameterSet> Ptr;
  typedef boost::tuple<std::string, Parameter*> SingleEntry;
  typedef std::vector< boost::tuple<std::string, Parameter*> > EntryList;

public:
  ParameterSet();
  ParameterSet ( const ParameterSet& o );
  ParameterSet ( const EntryList& entries );
  virtual ~ParameterSet();

  void operator=(const ParameterSet& o);

  EntryList entries() const;

  /**
   * insert values from entries, that are not present.
   * Do not overwrite entries!
   */
  void extend ( const EntryList& entries );

  /**
   * insert values from other, overwrite where possible.
   * return a non-const reference to this PS to anable call chains like PS.merge().merge()...
   */
  ParameterSet& merge ( const ParameterSet& other );

  template<class T>
  T& get ( const std::string& name );

  template<class T>
  const T& get ( const std::string& name ) const
  {
      return const_cast<ParameterSet&>(*this).get<T>(name);
  }

  template<class T>
  const typename T::value_type& getOrDefault ( const std::string& name, const typename T::value_type& defaultValue ) const
  {
    try
      {
        return this->get<T> ( name ) ();
      }
    catch ( const std::exception& /*e*/ )
      {
        return defaultValue;
      }
  }

  inline bool contains ( const std::string& name ) const
  {
    const_iterator i = find ( name );
    return ( i!=end() );
  }

  inline int& getInt ( const std::string& name )
  {
    return this->get<IntParameter> ( name ) ();
  }
  
  inline double& getDouble ( const std::string& name )
  {
    return this->get<DoubleParameter> ( name ) ();
  }
  
  inline bool& getBool ( const std::string& name )
  {
    return this->get<BoolParameter> ( name ) ();
  }
  
  inline std::string& getString ( const std::string& name )
  {
    return this->get<StringParameter> ( name ) ();
  }
  
  inline arma::mat& getVector ( const std::string& name )
  {
    return this->get<VectorParameter> ( name ) ();
  }

  inline arma::mat& getMatrix ( const std::string& name )
  {
    return this->get<MatrixParameter> ( name ) ();
  }

  inline std::istream& getFileStream ( const std::string& name )
  {
    return this->get<PathParameter> ( name ) .stream();
  }

  inline ParameterSet& setInt ( const std::string& name, int v )
  {
    this->get<IntParameter> ( name ) () = v;
    return *this;
  }
  
  inline ParameterSet& setDouble ( const std::string& name, double v )
  {
    this->get<DoubleParameter> ( name ) () = v;
    return *this;
  }
  
  inline ParameterSet& setBool ( const std::string& name, bool v )
  {
    this->get<BoolParameter> ( name ) () = v;
    return *this;
  }
  
  inline ParameterSet& setString ( const std::string& name, const std::string& v )
  {
    this->get<StringParameter> ( name ) () = v;
    return *this;
  }
  
  inline ParameterSet& setVector ( const std::string& name, const arma::mat& v )
  {
    this->get<VectorParameter> ( name ) () = v;
    return *this;
  }

  inline ParameterSet& setMatrix ( const std::string& name, const arma::mat& m )
  {
    this->get<MatrixParameter> ( name ) () = m;
    return *this;
  }

  inline ParameterSet& setOriginalFileName ( const std::string& name, const boost::filesystem::path& fp)
  {
    this->get<PathParameter> ( name ).setOriginalFilePath(fp);
    return *this;
  }

  ParameterSet& getSubset ( const std::string& name );

  inline const int& getInt ( const std::string& name ) const
  {
    return this->get<IntParameter> ( name ) ();
  }
  
  inline const double& getDouble ( const std::string& name ) const
  {
    return this->get<DoubleParameter> ( name ) ();
  }
  
  inline const bool& getBool ( const std::string& name ) const
  {
    return this->get<BoolParameter> ( name ) ();
  }
  
  inline const std::string& getString ( const std::string& name ) const
  {
    return this->get<StringParameter> ( name ) ();
  }
  
  inline const arma::mat& getVector ( const std::string& name ) const
  {
    return this->get<VectorParameter> ( name ) ();
  }
  
  inline const boost::filesystem::path getPath ( const std::string& name, const boost::filesystem::path& basePath = "" ) const
  {
    return this->get<PathParameter> ( name ) .filePath(basePath);
  }
  
  const ParameterSet& getSubset ( const std::string& name ) const;
  
  inline const ParameterSet& operator[] ( const std::string& name ) const
  {
    return getSubset ( name );
  }

  inline void replace ( const std::string& key, Parameter* newp )
  {
    using namespace boost;
    using namespace boost::algorithm;

    if ( boost::contains ( key, "/" ) )
      {
        std::string prefix = copy_range<std::string> ( *make_split_iterator ( key, first_finder ( "/" ) ) );
        std::string remain = key;
        erase_head ( remain, prefix.size()+1 );
//        std::cout<<prefix<<" >> "<<remain<<std::endl;
        return this->getSubset ( prefix ).replace ( remain, newp );
      }
    else
      {
        this->find(key)->second.reset(newp);
//        boost::ptr_map<std::string, Parameter>::replace ( this->find ( key ), newp );
      }
  }

  /**
   * set selection of selectable subset parameter to typename of T and its dictionary to the parameters of p
   */
  template<class T>
  ParameterSet& setSelectableSubset(const std::string& key, const typename T::Parameters& p);


  virtual std::string latexRepresentation() const;
  virtual std::string plainTextRepresentation(int indent=0) const;

  virtual ParameterSet* cloneParameterSet() const;

  virtual void appendToNode ( rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
                              boost::filesystem::path inputfilepath ) const;
  virtual void readFromNode ( rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
                              boost::filesystem::path inputfilepath );


  void packExternalFiles();
  void removePackedData();

  virtual void saveToStream(std::ostream& os, const boost::filesystem::path& parentPath, std::string analysisName = std::string() ) const;
  void saveToFile ( const boost::filesystem::path& file, std::string analysisType = std::string() ) const;
  virtual std::string readFromFile ( const boost::filesystem::path& file );

};


#ifndef SWIG
std::ostream& operator<<(std::ostream& os, const ParameterSet& ps);
#endif


typedef std::shared_ptr<ParameterSet> ParameterSetPtr;

#define PSINT(p, subdict, key) int key = p[subdict].getInt(#key);
#define PSDBL(p, subdict, key) double key = p[subdict].getDouble(#key);
#define PSSTR(p, subdict, key) std::string key = p[subdict].getString(#key);
#define PSBOOL(p, subdict, key) bool key = p[subdict].getBool(#key);
#define PSPATH(p, subdict, key) boost::filesystem::path key = p[subdict].getPath(#key);




class SubsetParameter
  : public Parameter,
    public ParameterSet //std::shared_ptr<ParameterSet>
{
public:
  typedef std::shared_ptr<SubsetParameter> Ptr;
  typedef ParameterSet value_type;

// protected:
//   std::shared_ptr<ParameterSet> value_;

public:
  declareType ( "subset" );

  SubsetParameter();
  SubsetParameter ( const std::string& description,  bool isHidden=false, bool isExpert=false, bool isNecessary=false, int order=0 );
  SubsetParameter ( const ParameterSet& defaultValue, const std::string& description,  bool isHidden=false, bool isExpert=false, bool isNecessary=false, int order=0 );

  inline void setParameterSet ( const ParameterSet& paramset )
  {
//     /*value_.*/this->reset(paramset.clone());
    this->setParameterSet ( paramset );
  }
//   void merge(const SubsetParameter& other);
  inline ParameterSet& operator() ()
  {
    return /**value_*/ static_cast<ParameterSet&> ( *this );
  }
  inline const ParameterSet& operator() () const
  {
    return /**value_*/static_cast<const ParameterSet&> ( *this );
  }

  std::string latexRepresentation() const override;
  std::string plainTextRepresentation(int indent=0) const override;

  bool isPacked() const override;
  void pack() override;
  void unpack(const boost::filesystem::path& basePath) override;
  void clearPackedData() override;


  rapidxml::xml_node<>* appendToNode
  (
      const std::string& name,
      rapidxml::xml_document<>& doc,
      rapidxml::xml_node<>& node,
      boost::filesystem::path inputfilepath
  ) const override;

  void readFromNode
  (
      const std::string& name,
      rapidxml::xml_document<>& doc,
      rapidxml::xml_node<>& node,
      boost::filesystem::path inputfilepath
  ) override;

  Parameter* clone () const override;

};




class SelectableSubsetParameter
  : public Parameter
{
public:
  typedef std::string key_type;
//  typedef boost::ptr_map<key_type, ParameterSet> ItemList;
  typedef std::map<key_type, std::unique_ptr<ParameterSet> > ItemList;
  typedef ItemList value_type;

  typedef boost::tuple<key_type, ParameterSet*> SingleSubset;
  typedef std::vector< boost::tuple<key_type, ParameterSet*> > SubsetList;

protected:
  key_type selection_;
  ItemList value_;

public:
  declareType ( "selectableSubset" );

  SelectableSubsetParameter ( const std::string& description,  bool isHidden=false, bool isExpert=false, bool isNecessary=false, int order=0 );
  /**
   * Construct from components:
   * \param defaultSelection The key of the subset which is selected per default
   * \param defaultValue A map of key-subset pairs. Between these can be selected
   * \param description The description of the selection parameter
   */
  SelectableSubsetParameter ( const key_type& defaultSelection, const SubsetList& defaultValue, const std::string& description,  bool isHidden=false, bool isExpert=false, bool isNecessary=false, int order=0 );

  inline key_type& selection()
  {
    return selection_;
  }
  
  inline const key_type& selection() const
  {
    return selection_;
  }
  
  inline const ItemList& items() const
  {
    return value_;
  }

  inline ItemList& items()
  {
    return value_;
  }

  void addItem ( key_type key, const ParameterSet& ps );
  
  inline ParameterSet& operator() ()
  {
    return * ( value_.find ( selection_ )->second );
  }
  
  inline const ParameterSet& operator() () const
  {
    return * ( value_.find ( selection_ )->second );
  }
  
  void setSelection(const key_type& key, const ParameterSet& ps);

  std::string latexRepresentation() const override;
  std::string plainTextRepresentation(int indent=0) const override;

  bool isPacked() const override;
  void pack() override;
  void unpack(const boost::filesystem::path& basePath) override;
  void clearPackedData() override;


  rapidxml::xml_node<>* appendToNode ( const std::string& name, rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
      boost::filesystem::path inputfilepath ) const override;
  void readFromNode ( const std::string& name, rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
                              boost::filesystem::path inputfilepath ) override;

  Parameter* clone () const override;
  void reset(const Parameter& p) override;

};




template<class T>
ParameterSet& ParameterSet::setSelectableSubset(const std::string& key, const typename T::Parameters& p)
{
    SelectableSubsetParameter& ssp = this->get<SelectableSubsetParameter>(key);
    ssp.selection()=p.type();
    ssp().merge(p);
    return *this;
}

  template<class T>
  T& ParameterSet::get ( const std::string& name )
  {
    using namespace boost;
    using namespace boost::algorithm;
    typedef T PT;

    if ( boost::contains ( name, "/" ) )
      {
        std::string prefix = copy_range<std::string> ( *make_split_iterator ( name, first_finder ( "/" ) ) );
        std::string remain=name;
        erase_head ( remain, prefix.size()+1 );
        
        std::vector<std::string> path;
        boost::split(path, name, boost::is_any_of("/"));
        insight::ArrayParameter* ap=NULL;
        if (path.size()>=2)
        {
            if ( this->find(path[0]) != this->end() ) 
                ap=dynamic_cast<insight::ArrayParameter*> ( find(path[0])->second.get() );
        }
        
        if (ap)
        {
            int i=boost::lexical_cast<int>(path[1]);
            if (path.size()==2)
            {
                PT* pt=dynamic_cast<PT*> ( &ap[i] );
                if ( pt )
                    return ( *pt );
                else
                {
                    throw insight::Exception ( "Parameter "+name+" not of requested type!" );
                }
            }
            else
            {
                std::string key=accumulate
                (
                    path.begin()+2, 
                    path.end(), 
                    std::string(),
                    [](std::string &ss, std::string &s)
                    {
                        return ss.empty() ? s : ss + "/" + s;
                    }
                );

                if (SubsetParameter* sps=dynamic_cast<SubsetParameter*>(&(*ap)[i]))
                {
                    return sps->get<T> ( key );
                }
                else
                if (SelectableSubsetParameter* sps=dynamic_cast<SelectableSubsetParameter*>(&(*ap)[i]))
                {
                    return (*sps)().get<T> ( key );
                }
                else
                {
                    throw insight::Exception ( "Array "+path[0]+" does not contain parameter sets!" );
                }
            }
        }
        else
            return this->getSubset ( prefix ).get<T> ( remain );
      }
    else
      {
        iterator i = find ( name );
        if ( i==end() )
          {
            throw insight::Exception ( "Parameter "+name+" not found in parameterset" );
          }
        else
          {
            PT* const pt=dynamic_cast<PT* const> ( i->second.get() );
            if ( pt )
              return ( *pt );
            else
            {
              throw insight::Exception ( "Parameter "+name+" not of requested type!" );
            }
          }
      }
  }




  class ParameterSet_Validator
  {
  public:
      typedef std::map<std::string, std::string> WarningList;
      typedef std::map<std::string, std::string> ErrorList;

  protected:
      ParameterSet ps_;


      WarningList warnings_;
      ErrorList errors_;

  public:
      virtual ~ParameterSet_Validator();

      /**
       * @brief update
       * @param ps
       * checks a parameter set (needs to be customized by derivation for that)
       * Stores a copy of the parameter set and updates the warnings and errors list.
       * Needs to be called first in derived classes.
       */
      virtual void update(const ParameterSet& ps);

      virtual bool isValid() const;
      virtual const WarningList& warnings() const;
      virtual const ErrorList& errors() const;
  };


  typedef std::shared_ptr<ParameterSet_Validator> ParameterSet_ValidatorPtr;





  class ParameterSet_Visualizer
  {

    // not linked to CAD; don't use any non-forward definitions from CAD module
  private:
      TextProgressDisplayer defaultProgressDisplayer_;
  protected:
      ParameterSet ps_;
      ProgressDisplayer* progress_;

  public:
      ParameterSet_Visualizer();
      virtual ~ParameterSet_Visualizer();

      /**
       * @brief update
       * @param ps
       * updates the parameterset which is to visualize.
       * This triggers recomputation of visualization features (from insight::cad) for several parameters.
       */
      virtual void update(const ParameterSet& ps);

      virtual void setIcon(QIcon* icon);

      void setProgressDisplayer(ProgressDisplayer* pd);
  };

  typedef std::shared_ptr<ParameterSet_Visualizer> ParameterSet_VisualizerPtr;

}


#endif // INSIGHT_PARAMETERSET_H
