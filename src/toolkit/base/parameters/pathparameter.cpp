#include "pathparameter.h"

#include <streambuf>
#include <ostream>
#include <memory>

#include "vtkSTLWriter.h"

#include "base/tools.h"

namespace insight
{




defineType(PathParameter);
addToFactoryTable(Parameter, PathParameter);




PathParameter::PathParameter(
    const std::string& description,
    bool isHidden, bool isExpert, bool isNecessary, int order
    )
: Parameter(description, isHidden, isExpert, isNecessary, order)
{}




PathParameter::PathParameter(
    const boost::filesystem::path& value, const std::string& description,
    bool isHidden, bool isExpert, bool isNecessary, int order,
    std::shared_ptr<std::string> binary_content
    )
: Parameter(description, isHidden, isExpert, isNecessary, order),
  FileContainer(value, binary_content)
{}




PathParameter::PathParameter(
    const FileContainer& fc, const std::string& description,
    bool isHidden, bool isExpert, bool isNecessary, int order
    )
 :  Parameter(description, isHidden, isExpert, isNecessary, order),
    FileContainer(fc)
{}




std::string PathParameter::latexRepresentation() const
{
    return SimpleLatex( valueToString ( originalFilePath_ ) ).toLaTeX();
}




std::string PathParameter::plainTextRepresentation(int /*indent*/) const
{
  return SimpleLatex( valueToString ( originalFilePath_ ) ).toPlainText();
}




bool PathParameter::isPacked() const
{
//  return FileContainer::isPacked();
  return hasFileContent();
}




void PathParameter::pack()
{
//  FileContainer::pack()
  replaceContent(originalFilePath_);
}




void PathParameter::unpack(const boost::filesystem::path &basePath)
{
  auto up = unpackFilePath(basePath);
  if (needsUnpack(up)) copyTo(up, true);
//  FileContainer::unpack(basePath);
}




void PathParameter::clearPackedData()
{
  FileContainer::clearPackedData();
}




boost::filesystem::path PathParameter::filePath(boost::filesystem::path baseDirectory) const
{
  auto up=unpackFilePath(baseDirectory);

  if (needsUnpack(up))
  {
    copyTo(up, true);
  }

  return up;
}








//template <typename char_type>
//struct ostreambuf
//    : public std::basic_streambuf<char_type, std::char_traits<char_type> >
//{
//    ostreambuf(char_type* buffer, std::streamsize bufferLength)
//    {
//        // set the "put" pointer the start of the buffer and record it's length.
//        this->setp(buffer, buffer + bufferLength);
//    }
//};

rapidxml::xml_node<>* PathParameter::appendToNode
(
  const std::string& name,
  rapidxml::xml_document<>& doc,
  rapidxml::xml_node<>& node,
  boost::filesystem::path inputfilepath
) const
{
    using namespace rapidxml;
    xml_node<>* child = Parameter::appendToNode(name, doc, node, inputfilepath);

    FileContainer::appendToNode(
          doc, *child,
          inputfilepath
    );

    return child;

}

void PathParameter::readFromNode
(
  const std::string& name,
  rapidxml::xml_document<>& doc,
  rapidxml::xml_node<>& node,
  boost::filesystem::path inputfilepath
)
{
  using namespace rapidxml;
  xml_node<>* child = findNode(node, name, type());
  if (child)
  {
    FileContainer::readFromNode(doc, *child, inputfilepath);
  }
  else
  {
    insight::Warning(
          boost::str(
            boost::format(
             "No xml node found with type '%s' and name '%s', default value '%s' is used."
             ) % type() % name % originalFilePath_.string()
           )
        );
  }
}

PathParameter *PathParameter::clonePathParameter() const
{
  return new PathParameter(
        originalFilePath_,
        description_.simpleLatex(),
        isHidden_, isExpert_, isNecessary_, order_,
        file_content_);
}

Parameter* PathParameter::clone() const
{
  return clonePathParameter();
}

void PathParameter::reset(const Parameter& p)
{
  if (const auto* op = dynamic_cast<const PathParameter*>(&p))
  {
    Parameter::reset(p);
    file_content_=op->file_content_;
  }
  else
    throw insight::Exception("Tried to set a "+type()+" from a different type ("+p.type()+")!");
}




void PathParameter::operator=(const PathParameter &op)
{
  description_ = op.description_;
  isHidden_ = op.isHidden_;
  isExpert_ = op.isExpert_;
  isNecessary_ = op.isNecessary_;
  order_ = op.order_;

  originalFilePath_ = op.originalFilePath_;
  file_content_ = op.file_content_;
//  fileContentHash_=op.fileContentHash_;
}




void PathParameter::operator=(const FileContainer& oc)
{
  originalFilePath_ = oc.originalFilePath_;
  file_content_ = oc.file_content_;
//  fileContentHash_=oc.fileContentHash_;
}




std::shared_ptr<PathParameter> make_filepath(const boost::filesystem::path& path)
{
  return std::shared_ptr<PathParameter>(new PathParameter(path, "temporary file path"));
}




std::shared_ptr<PathParameter> make_filepath(const FileContainer& fc)
{
  return std::shared_ptr<PathParameter>(new PathParameter(fc, "temporary file path"));
}



std::shared_ptr<PathParameter> make_filepath(
    vtkSmartPointer<vtkPolyData> pd,
    const boost::filesystem::path& originalFilePath )
{
  TemporaryFile tf( originalFilePath.filename().stem().string()+"-%%%%%%.stl" );
  auto msw = vtkSmartPointer<vtkSTLWriter>::New();
  msw->SetInputData(pd);
  msw->SetFileTypeToBinary();
  msw->SetFileName(tf.path().string().c_str());
  msw->Update();
  return std::make_shared<PathParameter>(
        FileContainer(originalFilePath, tf),
        "temporary file path" );
}






defineType(DirectoryParameter);
addToFactoryTable(Parameter, DirectoryParameter);

DirectoryParameter::DirectoryParameter(const std::string& description,  bool isHidden, bool isExpert, bool isNecessary, int order)
: PathParameter(".", description, isHidden, isExpert, isNecessary, order)
{}

DirectoryParameter::DirectoryParameter(const boost::filesystem::path& value, const std::string& description,  bool isHidden, bool isExpert, bool isNecessary, int order)
: PathParameter(value, description, isHidden, isExpert, isNecessary, order)
{}

std::string DirectoryParameter::latexRepresentation() const
{
    return std::string()
      + "{\\ttfamily "
      + SimpleLatex( boost::lexical_cast<std::string>(boost::filesystem::absolute(originalFilePath_)) ).toLaTeX()
      + "}";
}

//std::string DirectoryParameter::plainTextRepresentation(int indent) const
//{
//    return std::string(indent, ' ')
//      + "\""
//      + SimpleLatex( boost::lexical_cast<std::string>(boost::filesystem::absolute(value_)) ).toPlainText()
//      + "\"\n";
//}


rapidxml::xml_node<>* DirectoryParameter::appendToNode(const std::string& name, rapidxml::xml_document<>& doc, rapidxml::xml_node<>& node,
    boost::filesystem::path inputfilepath) const
{
    using namespace rapidxml;
    xml_node<>* child = Parameter::appendToNode(name, doc, node, inputfilepath);
    child->append_attribute(doc.allocate_attribute
    (
      "value",
      doc.allocate_string(originalFilePath_.c_str())
    ));
    return child;
}

void DirectoryParameter::readFromNode
(
    const std::string& name,
    rapidxml::xml_document<>&,
    rapidxml::xml_node<>& node,
    boost::filesystem::path
)
{
  using namespace rapidxml;
  xml_node<>* child = findNode(node, name, type());
  if (child)
  {
    originalFilePath_=boost::filesystem::path(child->first_attribute("value")->value());
  }
  else
  {
    insight::Warning(
          boost::str(
            boost::format(
             "No xml node found with type '%s' and name '%s', default value '%s' is used."
             ) % type() % name % originalFilePath_.string()
           )
        );
  }}



Parameter* DirectoryParameter::clone() const
{
  return new DirectoryParameter(originalFilePath_, description_.simpleLatex(), isHidden_, isExpert_, isNecessary_, order_);
}


void DirectoryParameter::reset(const Parameter& p)
{
  if (const auto* op = dynamic_cast<const DirectoryParameter*>(&p))
  {
    PathParameter::reset(p);
  }
  else
    throw insight::Exception("Tried to set a "+type()+" from a different type ("+p.type()+")!");
}



}
