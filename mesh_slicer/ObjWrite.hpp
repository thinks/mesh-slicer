#ifndef OBJ_WRITE_HPP_INCLUDED
#define OBJ_WRITE_HPP_INCLUDED

#include <vector>
#include <vector>
#include <string>
#include <fstream>

namespace obj {

namespace detail {

class ofstream {
public:
  explicit
  ofstream(std::string const& fileName, 
           std::ios::openmode const mode = std::ios::out) {	
    _ofs.open(fileName.c_str(), mode);
  }

  ~ofstream() { 
    _ofs.close(); 
  }

  // Expose std::ifstream for operators, etc.
  std::ofstream& 
  stream() { 
    return _ofs; 
  }

private: // Member variables.
  std::ofstream _ofs;
};

template<typename T> inline
ofstream& 
operator<<(ofstream& ofs, T const& t) {
  ofs.stream() << t;
  return ofs;
}

} // Namespace: detail

template <class PositionT, class IndexT> inline
void
write(std::string const& fileName, 
      std::vector<PositionT> const& positions, 
      std::vector<IndexT> const& indices) 
{
  using detail::ofstream;
  using std::size_t;

  typedef PositionT Position;
  typedef IndexT Index;

  ofstream ofs(fileName);

  size_t const positionsSize = positions.size();
  for (size_t i = 0; i < positionsSize; ++i) {
    Position const& pos = positions[i];
    ofs << "v " << pos[0] << ' ' << pos[1] << ' ' << pos[2] << '\n';
  }

  ofs << '\n';

  size_t const indicesSize = indices.size();
  for (size_t i = 0; i < indicesSize; ++i) {
    Index const& index = indices[i];
    ofs << "f " << index[0] + 1 << ' ' 
                << index[1] + 1 << ' ' 
                << index[2] + 1 << '\n';
  }
}

} // Namespace: obj

#endif // OBJ_WRITE_HPP_INCLUDED
