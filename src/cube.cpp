#include "utils.hpp"
#include "cube.hh"
#include <cassert>

constexpr CornerPerm    eCP = {0,1,2,3,4,5,6,7};
constexpr EdgePerm      eEP = {0,1,2,3,4,5,6,7,8,9,10,11};
constexpr CornerOri     eCO = {0,0,0,0,0,0,0,0};
constexpr EdgeOri       eEO = {0,0,0,0,0,0,0,0,0,0,0,0};
constexpr FacePerm      eFP = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53};

const FaceCube FaceCube::id = FaceCube(eFP);
const CubieCube CubieCube::id = CubieCube(eCP,eCO,eEP,eEO);

FaceCube FaceCube::fromString(const std::string &s)
{
    if(!is_valid_config(s)) throw std::invalid_argument("Invaid cube string");

    FacePerm fp;
    for(int i = 0; i < 6; ++i) {
        fp[CC[i]] = CC[i];
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
    return FaceCube(fp);
}

std::string FaceCube::color(std::string cset) const
{
    assert(cset.size() >= 6);
    std::string cs(54,' ');
    for(int i = 0; i < 54; i++) { 
        cs[i] = cset[f[i]/9]; 
    }
    return cs;
}

FaceCube::FaceCube(const CubieCube &cc)
{
    for(int i = 0; i < 6; i++) { 
        f[CC[i]] = CC[i];
    }
    for(int i = 0; i < 8; i++) for(int j = 0; j < 3; j++) {
        f[CF[i][j]] = CF[cc.cp[i]][(j-cc.co[i]+3)%3];
    }
    for(int i = 0; i < 12; i++) for(int j = 0; j < 2; j++) {
        f[EF[i][j]] = EF[cc.ep[i]][(j-cc.eo[i]+2)%2];
    }
}

CubieCube::CubieCube(const FaceCube &fc)
{
    for(int i = 0; i < 8; i++) {
        for(int x = 0; x < 24; x++) {
            if(fc.f[CF[i][0]] == CF[x/3][x%3]) {
                cp[i] = x/3, co[i] = (3-x%3)%3;
                break;
            }
        }
    }
    for(int i = 0; i < 12; i++){
        for(int y = 0; y < 24; y++){
            if(fc.f[EF[i][0]] == EF[y/2][y%2]) {
                ep[i] = y/2, eo[i] = (2-y%2)%2;
                break;
            }
        }
    }
}