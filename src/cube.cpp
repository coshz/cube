#include <iostream>
#include <set>
#include "cube.hh"

constexpr CornerPerm    eCP = {0,1,2,3,4,5,6,7};
constexpr EdgePerm      eEP = {0,1,2,3,4,5,6,7,8,9,10,11};
constexpr CornerOri     eCO = {0,0,0,0,0,0,0,0};
constexpr EdgeOri       eEO = {0,0,0,0,0,0,0,0,0,0,0,0};
constexpr FacePerm      eFP = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53};

const FaceCube FaceCube::id = FaceCube(eFP);
const CubieCube CubieCube::id = CubieCube(eCP,eCO,eEP,eEO);

template<typename VectorLike>
bool isValidConfig(const VectorLike &cfg)
{
    // check size
    if(cfg.size() != 54) return false;
        
    // check centers
    std::set<char> vs { cfg[CC[0]],cfg[CC[1]],cfg[CC[2]],cfg[CC[3]],cfg[CC[4]],cfg[CC[5]] };
    if(vs.size() != 6) return false;

    // check cubies
    for(size_t i = 0, x = 0; i < 8; i++) {
        for(x = 0; x < 24; x++) {
            if(cfg[CC[CCI[i][0]]] == cfg[CF[x/3][x%3]] 
               && cfg[CC[CCI[i][1]]] == cfg[CF[x/3][(x+1)%3]]
               && cfg[CC[CCI[i][2]]] == cfg[CF[x/3][(x+2)%3]]) break;
        }
        if(x >= 24) return false;
    }
    for(size_t i = 0, y = 0; i < 12; i++) {
        for(y = 0; y < 24; y++) {
            if(cfg[CC[ECI[i][0]]] == cfg[EF[y/2][y%2]]
               && cfg[CC[ECI[i][1]]] == cfg[EF[y/2][(y+1)%2]]) break;
        }
        if(y >= 24) return false;
    }
    return true;
}

FaceCube FaceCube::fromString(const std::string &s)
{
    if(!isValidConfig(s)) throw std::invalid_argument("Invaid cube string");

    FaceCube fc;
    for(size_t i = 0, x = 0; i < 8; i++) {
        for(x = 0; x < 24; x++) {
            if(s[CC[CCI[i][0]]] == s[CF[x/3][x%3]]
            && s[CC[CCI[i][1]]] == s[CF[x/3][(x+1)%3]]
            && s[CC[CCI[i][2]]] == s[CF[x/3][(x+2)%3]]) 
            {
                fc.f[CF[x/3][x%3]]      = CF[i][0];
                fc.f[CF[x/3][(x+1)%3]]  = CF[i][1];
                fc.f[CF[x/3][(x+2)%3]]  = CF[i][2];
                break;
            }
        }
    }
    for(size_t i = 0, y = 0; i < 12; i++) {
        for(y = 0; y < 24; y++) {
            if(s[CC[ECI[i][0]]] == s[EF[y/2][y%2]]
            && s[CC[ECI[i][1]]] == s[EF[y/2][(y+1)%2]])
            {
                fc.f[EF[y/2][y%2]]      = EF[i][0];
                fc.f[EF[y/2][(y+1)%2]]  = EF[i][1];
                break;
            }
        }
    }
    return fc;
}

std::string FaceCube::color(std::string cset) const
{
    assert(cset.size() >= 6);
    std::string cs(54,' ');
    for(size_t i = 0; i < 54; i++) cs[i] = cset[this->f[i]/9];
    return cs;
}

FaceCube::FaceCube(const CubieCube &cc)
{
    for(size_t i = 0; i < 6; i++) { 
        this->f[CC[i]] = CC[i]; 
    }
    for(size_t i = 0; i < 8; i++) for(size_t j = 0; j < 3; j++) {
        this->f[CF[i][j]] = CF[cc.cp[i]][(3 - cc.co[i] + j) % 3];
    }
    for(size_t i = 0; i < 12; i++) for(size_t j = 0; j < 2; j++) {
        this->f[EF[i][j]] = EF[cc.ep[i]][(2 - cc.eo[i] + j) % 2];
    }
}

CubieCube CubieCube::fromString(const std::string &s)
{
    return CubieCube(FaceCube::fromString(s));
}

CubieCube::CubieCube(const FaceCube &fc)
{
    for(size_t i = 0, x = 0; i < 8; i++) {
        for(x = 0; x < 24; x++) {
            if(fc.f[CF[i][0]] == CF[x/3][x%3]) {
                this->cp[i] = x/3, this->co[i] =(3-x%3)%3; 
                break;
            } 
        }
        assert(x < 24);
    }
    for(size_t i = 0, y = 0; i < 12; i++){
        for(y = 0; y < 24; y++){
            if(fc.f[EF[i][0]] == EF[y/2][y%2]) {
                this->ep[i] = y/2, this->eo[i] = (2-y%2)%2;
                break;
            }
        }
        assert(y < 24);
    }
}
