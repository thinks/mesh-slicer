#ifndef OBJ_READ_HPP_INCLUDED
#define OBJ_READ_HPP_INCLUDED

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

namespace obj {

namespace detail {

class ifstream {
public:
  explicit
  ifstream(std::string const& fileName, 
           std::ios::openmode const mode = std::ios::in) {	
    _ifs.open(fileName.c_str(), mode);
  }

  ~ifstream() { 
    _ifs.close(); 
  }

  // Expose std::ifstream for operators, etc.
  std::ifstream& 
  stream() { 
    return _ifs; 
  }

private: // Member variables.
  std::ifstream _ifs;
};

//! DOCS
template <typename T> inline
T 
lexical_cast(std::string const& str) {
  T t;
  std::istringstream iss;
  iss.str(str);
  iss >> t;
  // TODO: deal with any error bits that may have been set on the stream.
  return t;
}

//! DOCS
std::vector<std::string> 
tokenize(std::string const& str, char const delim) {
  std::stringstream ss(str);
  std::string token;
  std::vector<std::string> tokens;
  while (std::getline(ss, token, delim)) {
    tokens.push_back(token);
  }
  return tokens;
}

} // Namespace: detail

//! DOCS
template<class PositionT, class IndexT> inline 
void 
read(std::string const& fileName, 
     std::vector<PositionT>& positions, 
     std::vector<IndexT>& indices) 
{
  using detail::ifstream;
  using detail::lexical_cast;
  using detail::tokenize;
  using std::vector;
  using std::string;
  using std::getline;
  using std::size_t;

  typedef PositionT Position;
  typedef typename Position::value_type PositionElement;
  typedef IndexT Index;
  typedef typename Index::value_type IndexElement;

  positions.clear();
  indices.clear();
  ifstream ifs(fileName);
  string line;
  while (getline(ifs.stream(), line, '\n')) {
    vector<string> const lineTokens = tokenize(line, ' ');
    if (lineTokens.size() == 4) {
      if (lineTokens[0] == "v") {
        positions.push_back(
          Position(
            lexical_cast<PositionElement>(lineTokens[1]),
            lexical_cast<PositionElement>(lineTokens[2]),
            lexical_cast<PositionElement>(lineTokens[3])));
      }
      else if (lineTokens[0] == "f") {
        Index index;
        for (size_t i = 0; i < 3; ++i) {
          vector<string> const indexTokens = tokenize(lineTokens[i + 1], '/');
          index[i] = lexical_cast<IndexElement>(indexTokens[0]) - 1;
        }
        indices.push_back(index);
      }
    }
  }
}

} // Namespace: obj

#endif // OBJ_READ_HPP_INCLUDED
