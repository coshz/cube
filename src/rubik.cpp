#include "utils.hpp"
#include "rubik.hh"
#include <cassert>

namespace cube {

ColorState ColorState::fromString(std::string_view cube)
{
    assert(is_valid_config(cube) && "invalid cube config");
   
    std::array<Layer,54> xs{};
    std::transform(cube.cbegin(), cube.cend(), xs.begin(), [](char c) {
        switch(c) {
        case 'U': return U;
        case 'R': return R;
        case 'F': return F;
        case 'D': return D;
        case 'L': return L;
        case 'B': return B;
        default: throw std::invalid_argument("invalid cube config");
        }
    });
    return { xs };
}

FaceCube ColorState::toFaceCube() const
{
    FacePerm fp{};
    for(int i = 0; i < 6; ++i) {
        fp[CC[i]] = s[CC[i]];
    }
    for(int i = 0; i < 8; i++) {
        for(int x = 0; x < 24; x++) {
            if(s[CC[CCI[i][0]]] == s[CF[x/3][x%3]]
            && s[CC[CCI[i][1]]] == s[CF[x/3][(x+1)%3]]
            && s[CC[CCI[i][2]]] == s[CF[x/3][(x+2)%3]])
            {
                fp[CF[x/3][x%3]]      = CF[i][0];
                fp[CF[x/3][(x+1)%3]]  = CF[i][1];
                fp[CF[x/3][(x+2)%3]]  = CF[i][2];
                break;
            }
        }
    }
    for(int i = 0; i < 12; i++) {
        for(int y = 0; y < 24; y++) {
            if(s[CC[ECI[i][0]]] == s[EF[y/2][y%2]]
            && s[CC[ECI[i][1]]] == s[EF[y/2][(y+1)%2]])
            {
                fp[EF[y/2][y%2]]      = EF[i][0];
                fp[EF[y/2][(y+1)%2]]  = EF[i][1];
                break;
            }
        }
    }
    return { fp };
}

CubieCube ColorState::toCubieCube() const
{
    return toFaceCube().toCubieCube();
}

CubieCube FaceCube::toCubieCube() const
{
    CubieCube cc{};
    const FaceCube &fc = *this;

    for(int i = 0; i < 8; i++) {
        for(int x = 0; x < 24; x++) {
            if(fc.f[CF[i][0]] == CF[x/3][x%3]) {
                cc.cp[i] = x/3, cc.co[i] = (3-x%3)%3;
                break;
            }
        }
    }
    for(int i = 0; i < 12; i++){
        for(int y = 0; y < 24; y++){
            if(fc.f[EF[i][0]] == EF[y/2][y%2]) {
                cc.ep[i] = y/2, cc.eo[i] = (2-y%2)%2;
                break;
            }
        }
    }
    return cc;
}

FaceCube CubieCube::toFaceCube() const
{
    FaceCube fc{};
    const CubieCube &cc = *this;

    for(int i = 0; i < 6; i++) {
        fc.f[CC[i]] = CC[i];
    }
    for(int i = 0; i < 8; i++) for(int j = 0; j < 3; j++) {
        fc.f[CF[i][j]] = CF[cc.cp[i]][(j-cc.co[i]+3)%3];
    }
    for(int i = 0; i < 12; i++) for(int j = 0; j < 2; j++) {
        fc.f[EF[i][j]] = EF[cc.ep[i]][(j-cc.eo[i]+2)%2];
    }
    return fc;
}

} // namespace cube