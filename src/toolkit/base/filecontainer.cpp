
#include "filecontainer.h"


#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/binary_from_base64.hpp"
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>


#include "base/tools.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


namespace boost
{
namespace filesystem
{




template < >
// path& path::append< typename path::iterator >( typename path::iterator begin, typename path::iterator end, const codecvt_type& cvt)
path&
path::append< typename path::iterator >(
    typename path::iterator begin,
    typename path::iterator end )
{
  for( ; begin != end ; ++begin )
    *this /= *begin;
  return *this;
}



// Return path when appended to a_From will resolve to same as a_To
path
make_relative(
    path a_From,
    path a_To )
{
  a_From = absolute( a_From ); a_To = absolute( a_To );
  path ret;
  path::const_iterator itrFrom( a_From.begin() ), itrTo( a_To.begin() );

  // Find common base
  for(
      path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ;
      itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo;
      ++itrFrom, ++itrTo );

  // Navigate backwards in directory to reach previously found base
  for( path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom )
  {
    if( (*itrFrom) != "." )
      ret /= "..";
  }

  // Now navigate down the directory branch
  ret.append( itrTo, a_To.end() );
  return ret;
}


}
}




namespace insight {







const std::string base64_padding[] = {"", "==","="};




std::string base64_encode(const std::string& s)
{
  namespace bai = boost::archive::iterators;

  std::stringstream os;

  // convert binary values to base64 characters
  typedef bai::base64_from_binary
  // retrieve 6 bit integers from a sequence of 8 bit bytes
  <bai::transform_width<const char *, 6, 8> > base64_enc; // compose all the above operations in to a new iterator

  std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
            std::ostream_iterator<char>(os));

  os << base64_padding[s.size() % 3];
  return os.str();
}




std::string
base64_encode(
    const boost::filesystem::path& f )
{
  std::string contents_raw;
  readFileIntoString(f, contents_raw);
  return base64_encode(contents_raw);
}




MD5HashPtr calcBufferHash(const std::string& buffer)
{
  insight::CurrentExceptionContext ex(
        str(boost::format("computing MD5 hash of buffer (size %d bytes)")%buffer.size()) );

  auto hash=std::make_shared<MD5Hash>();

  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, buffer.data(), buffer.size());
  MD5_Final(hash->data(), &ctx);

  return hash;
}



MD5HashPtr calcFileHash(const boost::filesystem::path& filePath)
{
  insight::CurrentExceptionContext ex("computing MD5 hash of file "+filePath.string());

  auto hash=std::make_shared<MD5Hash>();

  int file_descript;
  unsigned long file_size;
  char* file_buffer;

  file_descript = open(filePath.c_str(), O_RDONLY);
  if(file_descript < 0) exit(-1);

  struct stat statbuf;
  if(fstat(file_descript, &statbuf) < 0)
    throw insight::Exception("Failed to get file attributes of file "+filePath.string());
  file_size=statbuf.st_size;

  file_buffer = static_cast<char*>(
        mmap(
          0, file_size,
          PROT_READ, MAP_SHARED,
          file_descript, 0 )
        );

  MD5( reinterpret_cast<unsigned char*>(file_buffer),
       file_size,
       hash->data() );

  munmap(file_buffer, file_size);

  return hash;
}




void
base64_decode(
    const char *src,
    size_t size,
    std::shared_ptr<std::string>& targetBuffer  )
{
//  char *src = a->value();
//  size_t size = a->value_size();

  if ((size>0) && src[size - 1] == '=')
  {
    --size;
    if ((size>0) && src[size - 1] == '=')
    {
       --size;
    }
  }

  if (size == 0)
  {
    if (targetBuffer) targetBuffer->clear();
  }
  else
  {
    using namespace boost::archive::iterators;

    typedef
      transform_width<
       binary_from_base64<
        remove_whitespace<
         const char*
        >
       >,
       8, 6
      >
      base64_dec;

      targetBuffer.reset(new std::string( base64_dec(src), base64_dec(src + size) ));
  }
}




bool operator<(const timespec& lhs, const timespec& rhs)
{
    if (lhs.tv_sec == rhs.tv_sec)
        return lhs.tv_nsec < rhs.tv_nsec;
    else
        return lhs.tv_sec < rhs.tv_sec;
}


bool operator==(const timespec& lhs, const timespec& rhs)
{
    return
        (lhs.tv_sec == rhs.tv_sec) &&
        (lhs.tv_nsec == rhs.tv_nsec);
}


std::ostream& operator<<(std::ostream& os, const timespec& ts)
{
  os<<ts.tv_sec<<"."<< std::setfill('0') << std::setw(9)<<ts.tv_nsec;
  return os;
}



boost::filesystem::path FileContainer::unpackFilePath(boost::filesystem::path baseDir) const
{
  if (!hasFileContent() || boost::filesystem::exists(originalFilePath_))
  {
    return originalFilePath_;
  }
  else
  {
    if (baseDir.empty())
    {
      baseDir=GlobalTemporaryDirectory::path();
    }
    if (!boost::filesystem::exists(baseDir))
    {
      throw insight::Exception("Error while trying to unpack embedded file: target path "+baseDir.string()+"does not exist!");
    }

    auto dirHash = std::hash<std::string>()(originalFilePath_.parent_path().string());
    auto rp = baseDir / "embeddedFiles" / boost::lexical_cast<std::string>(dirHash) / fileName();

    return rp;
  }
}




FileContainer::FileContainer()
{}




FileContainer::FileContainer(const FileContainer& other)
  : originalFilePath_(other.originalFilePath_),
    file_content_(other.file_content_), // get reference to the same content for performance reasons
    fileContentTimestamp_(other.fileContentTimestamp_)/*,
    fileContentHash_(other.fileContentHash_)*/
{
}




FileContainer::FileContainer(
    const boost::filesystem::path& originalFileName,
    std::shared_ptr<std::string> binary_content )
  : originalFilePath_(originalFileName),
    file_content_(binary_content)

{
  clock_gettime(CLOCK_REALTIME, &fileContentTimestamp_);
}




FileContainer::FileContainer(
    const boost::filesystem::path &originalFileName,
    const TemporaryFile &tf )
  : originalFilePath_(originalFileName)
{
  replaceContent(tf.path());
}




FileContainer::FileContainer(
    const char *binaryFileContent, size_t size,
    const boost::filesystem::path &originalFileName )
  : originalFilePath_(originalFileName),
    file_content_(new std::string(binaryFileContent, size))
{
  clock_gettime(CLOCK_REALTIME, &fileContentTimestamp_);
}



bool FileContainer::hasFileContent() const
{
  return bool(file_content_);
}




bool FileContainer::isValid() const
{
  return
      !originalFilePath_.empty() &&
      ( boost::filesystem::exists(originalFilePath_) || file_content_ ) ;
}



//boost::filesystem::path FileContainer::filePath(boost::filesystem::path baseDir) const
//{
//  if (hasFileContent())
//  {
//    const_cast<FileContainer*>(this)->unpack(baseDir); // does nothing, if already unpacked
//    return unpackFilePath(baseDir);
//  }

//  return originalFilePath_;
//}

const boost::filesystem::path &FileContainer::originalFilePath() const
{
  return originalFilePath_;
}

boost::filesystem::path FileContainer::fileName() const
{
  return originalFilePath_.filename();
}


void FileContainer::setOriginalFilePath(const boost::filesystem::path &value)
{
  originalFilePath_=value;
  clearPackedData();
}




std::istream& FileContainer::stream() const
{
  if (hasFileContent())
  {
    file_content_stream_.reset(new std::istringstream(*file_content_));
  }
  else
  {
    file_content_stream_.reset(new std::ifstream(originalFilePath_.c_str()));
    if (! (*file_content_stream_) )
    {
      throw insight::Exception("Could not open file "+originalFilePath_.string()+" for reading!");
    }
  }

  return *file_content_stream_;
}




const char *FileContainer::binaryFileContent() const
{
  insight::assertion(bool(file_content_), "There is no file content in memory");
  return file_content_->c_str();
}




//bool FileContainer::isPacked() const
//{
//  return bool(file_content_);
//}

//void FileContainer::pack()
//{
//  // read and store file
//  replaceContent(originalFilePath_);
//}

bool FileContainer::needsUnpack(const boost::filesystem::path& unpackPath) const
{
  if (!originalFilePath_.empty())
  {
    bool needUnpack = false;

    if (file_content_) // unpack, if we have content
    {
      if (!exists(unpackPath)) // unpack only, if not already done
      {
        needUnpack = true;
      }
      else
      {
        // only consider unpacking, if the data we have is newer than what is on disk
        int file_descript = open(unpackPath.c_str(), O_RDONLY);
        struct stat statbuf;
        if(fstat(file_descript, &statbuf) < 0)
          throw insight::Exception("Failed to get file attributes of file "+unpackPath.string());
        auto last_write_time = statbuf.st_mtim;

        if (last_write_time < fileContentTimestamp_)
        {
//          // check if the file on disk is actually different
//          auto cmd5 = calcFileHash(unpackPath);
//          if (*cmd5 != *fileContentHash_)
//          {
            needUnpack = true;
//          }
        }
      }
    }

    return needUnpack;
  }
  else
  {
    return false;
  }
}


//void FileContainer::unpack(const boost::filesystem::path& basePath)
//{
//  if (needsUnpack(basePath))
//  {

//    // extract file, create parent path.
//    if (!exists(unpackPath.parent_path()) )
//    {
//      boost::filesystem::create_directories( unpackPath.parent_path() );
//    }

//    std::ofstream file( unpackPath.c_str(), std::ios::out | std::ios::binary);
//    if (file.good())
//    {
//        file.write(file_content_->c_str(), long(file_content_->size()) );
//        file.close();
//    }

//  }
//}


void FileContainer::copyTo(const boost::filesystem::path &filePath, bool createParentPath) const
{
  if (createParentPath)
  {
    if (!exists(filePath.parent_path()) )
    {
      boost::filesystem::create_directories( filePath.parent_path() );
    }
  }

  if (file_content_)
  {
    std::ofstream file( filePath.c_str(), std::ios::out | std::ios::binary);
    if (file.good())
    {
        file.write(file_content_->c_str(), long(file_content_->size()) );
        file.close();
    }
    else
    {
      throw insight::Exception("could not write to file "+filePath.string());
    }
  }
  else
  {
    boost::filesystem::copy_file(originalFilePath_, filePath, boost::filesystem::copy_option::overwrite_if_exists);
  }
}


void FileContainer::replaceContent(const boost::filesystem::path& filePath)
{
  if (exists(filePath) && is_regular_file(filePath) )
  {
    // read raw file into buffer
    auto newContent = std::make_shared<std::string>();

    std::ifstream in(filePath.c_str());
    std::istreambuf_iterator<char> inputBegin(in), inputEnd;
    std::back_insert_iterator<std::string> stringInsert(*newContent);

    copy(inputBegin, inputEnd, stringInsert);

    replaceContentBuffer(newContent);
  }
}


void FileContainer::replaceContentBuffer(std::shared_ptr<std::string> newContent)
{
  file_content_=newContent;
  clock_gettime(CLOCK_REALTIME, &fileContentTimestamp_);
//  fileContentHash_ = calcBufferHash(*file_content_);
}




void FileContainer::clearPackedData()
{
  file_content_.reset();
  fileContentTimestamp_={0,0};
}




void FileContainer::appendToNode
(
    rapidxml::xml_document<> &doc,
    rapidxml::xml_node<> &node,
    boost::filesystem::path inputfilepath,
    const std::string& fileNameAttribName,
    const std::string& contentAttribName
) const
{
  using namespace rapidxml;

  std::string relpath="";
  if (!originalFilePath_.empty())
  {
    relpath=make_relative(inputfilepath, originalFilePath_).string();
  }
  node.append_attribute(doc.allocate_attribute
  (
    doc.allocate_string(fileNameAttribName.c_str()),
    doc.allocate_string(relpath.c_str())
  ));

  if (hasFileContent())
  {

    // ===========================================================================================
    // 1.) do base64 encode
    // typedefs
    using namespace boost::archive::iterators;
    typedef
//          insert_linebreaks<         // insert line breaks every 72 characters
            base64_from_binary<    // convert binary values to base64 characters
                transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                    const char*, 6, 8
                >
            >
//              ,72 >
        base64_enc; // compose all the above operations in to a new iterator

    char tail[3] = {0,0,0};
    size_t len=file_content_->size();
    uint one_third_len = len/3;
    uint len_rounded_down = one_third_len*3;
    uint j = len_rounded_down + one_third_len;
    unsigned int base64length = ((4 * file_content_->size() / 3) + 3) & ~3;

    auto *xml_content = doc.allocate_string(0, base64length+1);
    std::copy(
          base64_enc(file_content_->c_str()),
          base64_enc(file_content_->c_str()+len_rounded_down),
          xml_content
          );

    if (len_rounded_down != len)
    {
        uint i=0;
        for(; i < len - len_rounded_down; ++i)
        {
            tail[i] = (*file_content_)[len_rounded_down+i];
        }

        std::copy(base64_enc(tail), base64_enc(tail + 3), xml_content + j);

        for(i=len + one_third_len + 1; i < j+4; ++i)
        {
            xml_content[i] = '=';
        }
    }

    xml_content[base64length]=0;


    // ===========================================================================================
    // 2.) append to node
    node.append_attribute(doc.allocate_attribute
    (
      doc.allocate_string(contentAttribName.c_str()),
      //os.str().c_str()
      xml_content
    ));

  }
}


void FileContainer::readFromNode
(
  rapidxml::xml_document<> &doc,
  rapidxml::xml_node<>& node,
  boost::filesystem::path inputfilepath,
  const std::string& fileNameAttribName,
  const std::string& contentAttribName
)
{
  using namespace rapidxml;
  boost::filesystem::path abspath(node.first_attribute(doc.allocate_string(fileNameAttribName.c_str()))->value());

  if (!abspath.empty())
  {
    if (abspath.is_relative())
    {
      abspath = boost::filesystem::absolute(inputfilepath / abspath);
    }
#if BOOST_VERSION < 104800
#warning Conversion into canonical paths disabled!
#else
    if (boost::filesystem::exists(abspath)) // avoid throwing errors during read of parameterset
        abspath=boost::filesystem::canonical(abspath);
#endif
  }

  originalFilePath_=abspath;

  if (auto* a = node.first_attribute(doc.allocate_string(contentAttribName.c_str())))
  {
    base64_decode(a->value(), a->value_size(), file_content_);
    clock_gettime(CLOCK_REALTIME, &fileContentTimestamp_);
  }

}

const timespec &FileContainer::contentModificationTime() const
{
  return fileContentTimestamp_;
}

} // namespace insight
