#pragma once
/*
         U1 U2 U3 
         U4 U5 U6
         U7 U8 U9 
L1 L2 L3 F1 F2 F3 R1 R2 R3 B1 B2 B3
L4 L5 L6 F4 F5 F6 R4 R5 R6 B4 B5 B6
L7 L8 L9 F7 F8 F9 R7 R8 R9 B7 B8 B9
         D1 D2 D3
         D4 D5 D6
         D7 D8 D9
 */

#include <iostream>
enum Face       { U1,U2,U3,U4,U5,U6,U7,U8,U9,R1,R2,R3,R4,R5,R6,R7,R8,R9,F1,F2,F3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,D7,D8,D9,L1,L2,L3,L4,L5,L6,L7,L8,L9,B1,B2,B3,B4,B5,B6,B7,B8,B9 };
enum TurnAxis   { U,R,F,D,L,B };
enum TurnMove   { Ux1,Ux2,Ux3,Rx1,Rx2,Rx3,Fx1,Fx2,Fx3,Dx1,Dx2,Dx3,Lx1,Lx2,Lx3,Bx1,Bx2,Bx3 };
enum Symmetry   { S_URF3,S_F2,S_U4,S_LR2 };

enum ColorIndex { UCol,RCol,FCol,DCol,LCol,BCol,NoCol };
enum Corner     { URF,UFL,ULB,UBR,DFR,DLF,DBL,DRB };
enum Edge       { UR,UF,UL,UB,DR,DF,DL,DB,FR,FL,BL,BR };
struct OrientedCorner   { Corner c; unsigned o; };
struct OrientedEdge     { Edge e;   unsigned o; };

typedef ColorIndex      SingleFace[9];
typedef ColorIndex      CornerColorIndex[8][3];
typedef ColorIndex      EdgeColorIndex[12][2];
typedef ColorIndex      FaceletColor[54];
typedef Face            CentreFacelet[6];
typedef Face            CornerFacelet[8][3];
typedef Face            EdgeFacelet[12][2];
typedef Face            Facelet[54];
typedef OrientedCorner  CornerCubie[8];
typedef OrientedEdge    EdgeCubie[12];
typedef Corner          EdgeNeighbour[12][2];

const CornerColorIndex  CCI = {{UCol,RCol,FCol},{UCol,FCol,LCol},{UCol,LCol,BCol},{UCol,BCol,RCol},{DCol,FCol,RCol},{DCol,LCol,FCol},{DCol,BCol,LCol},{DCol,RCol,BCol}};
const EdgeColorIndex    ECI = {{UCol,RCol},{UCol,FCol},{UCol,LCol},{UCol,BCol},{DCol,RCol},{DCol,FCol}, {DCol,LCol},{DCol,BCol},{FCol,RCol},{FCol,LCol},{BCol,LCol},{BCol,RCol}};
const CentreFacelet     CC  = {U5,R5,F5,D5,L5,B5};
const CornerFacelet     CF  = {{U9,R1,F3},{U7,F1,L3},{U1,L1,B3},{U3,B1,R3},{D3,F9,R7},{D1,L9,F7},{D7,B9,L7},{D9,R9,B7}};
const EdgeFacelet       EF  = {{U6,R2},{U8,F2},{U4,L2},{U2,B2},{D6,R8},{D2,F8}, {D4,L8},{D8,B8},{F6,R4},{F4,L6},{B6,L4},{B4,R6}};
const EdgeNeighbour     EN  = {{URF,UBR},{UFL,URF},{ULB,UFL},{UBR,ULB},{DRB,DFR},{DFR,DLF}, {DLF,DBL},{DBL,DRB},{URF,DFR},{DLF,UFL},{DBL,ULB},{UBR,DRB}};

/*!
 * The schema of permutation p = (p[0],...,p[n-1]) (in P) 
 *  i.e., a bijection on {0,...,n-1}, is called:
 *  (1) "replaced by", if: p[i] = p(i)
 *  (2) "carry to",    if: p[i] = p'(i) (p' is the inverse of p)
 * Interpretation:
 *  We expect P acts on X freely, considering the operation of p on x:
 *  (1) replace by: 
 *      (X * p)[i] = X[p[i]]
 *      (p <*> q)[i] = p[q[i]]  -- (right action)
 *  (2) carry to: 
 *      (p . X)[p[i]] = X[i]
 *      (q <.> p)[i] = q[p[i]]  -- (left action)  
 * Symobols: 
 * From above, p <*> q = p <.> q denoting p * q, right acting denoting x * p and 
 * left acting denoting p * x.
 */

/* Facelet Move: using notation schema of "carry-to" */
const Facelet FaceletMove[6] = { 
    {U3,U6,U9,U2,U5,U8,U1,U4,U7,F1,F2,F3,R4,R5,R6,R7,R8,R9,L1,L2,L3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,D7,D8,D9,B1,B2,B3,L4,L5,L6,L7,L8,L9,R1,R2,R3,B4,B5,B6,B7,B8,B9}, 
    {U1,U2,B7,U4,U5,B4,U7,U8,B1,R3,R6,R9,R2,R5,R8,R1,R4,R7,F1,F2,U3,F4,F5,U6,F7,F8,U9,D1,D2,F3,D4,D5,F6,D7,D8,F9,L1,L2,L3,L4,L5,L6,L7,L8,L9,D9,B2,B3,D6,B5,B6,D3,B8,B9}, 
    {U1,U2,U3,U4,U5,U6,R1,R4,R7,D3,R2,R3,D2,R5,R6,D1,R8,R9,F3,F6,F9,F2,F5,F8,F1,F4,F7,L3,L6,L9,D4,D5,D6,D7,D8,D9,L1,L2,U9,L4,L5,U8,L7,L8,U7,B1,B2,B3,B4,B5,B6,B7,B8,B9}, 
    {U1,U2,U3,U4,U5,U6,U7,U8,U9,R1,R2,R3,R4,R5,R6,B7,B8,B9,F1,F2,F3,F4,F5,F6,R7,R8,R9,D3,D6,D9,D2,D5,D8,D1,D4,D7,L1,L2,L3,L4,L5,L6,F7,F8,F9,B1,B2,B3,B4,B5,B6,L7,L8,L9}, 
    {F1,U2,U3,F4,U5,U6,F7,U8,U9,R1,R2,R3,R4,R5,R6,R7,R8,R9,D1,F2,F3,D4,F5,F6,D7,F8,F9,B9,D2,D3,B6,D5,D6,B3,D8,D9,L3,L6,L9,L2,L5,L8,L1,L4,L7,B1,B2,U7,B4,B5,U4,B7,B8,U1}, 
    {L7,L4,L1,U4,U5,U6,U7,U8,U9,R1,R2,U1,R4,R5,U2,R7,R8,U3,F1,F2,F3,F4,F5,F6,F7,F8,F9,D1,D2,D3,D4,D5,D6,R9,R6,R3,D7,L2,L3,D8,L5,L6,D9,L8,L9,B3,B6,B9,B2,B5,B8,B1,B4,B7}
};

const Facelet FaceletSym[4] = {
    {R9,R8,R7,R6,R5,R4,R3,R2,R1,F3,F6,F9,F2,F5,F8,F1,F4,F7,U3,U6,U9,U2,U5,U8,U1,U4,U7,L1,L2,L3,L4,L5,L6,L7,L8,L9,B7,B4,B1,B8,B5,B2,B9,B6,B3,D3,D6,D9,D2,D5,D8,D1,D4,D7}, 
    {D9,D8,D7,D6,D5,D4,D3,D2,D1,L9,L8,L7,L6,L5,L4,L3,L2,L1,F9,F8,F7,F6,F5,F4,F3,F2,F1,U9,U8,U7,U6,U5,U4,U3,U2,U1,R9,R8,R7,R6,R5,R4,R3,R2,R1,B9,B8,B7,B6,B5,B4,B3,B2,B1}, 
    {U3,U6,U9,U2,U5,U8,U1,U4,U7,F1,F2,F3,F4,F5,F6,F7,F8,F9,L1,L2,L3,L4,L5,L6,L7,L8,L9,D7,D4,D1,D8,D5,D2,D9,D6,D3,B1,B2,B3,B4,B5,B6,B7,B8,B9,R1,R2,R3,R4,R5,R6,R7,R8,R9}, 
    {U3,U2,U1,U6,U5,U4,U9,U8,U7,L3,L2,L1,L6,L5,L4,L9,L8,L7,F3,F2,F1,F6,F5,F4,F9,F8,F7,D3,D2,D1,D6,D5,D4,D9,D8,D7,R3,R2,R1,R6,R5,R4,R9,R8,R7,B3,B2,B1,B6,B5,B4,B9,B8,B7} 
};

/*! 
 * @brief CornerCubie Move: using notation schema of "replace-by" 
 * @remark CornerCubieMove[m][i] = {c_i,o_i} means the `i`-th corner of identity 
 * cube is replaced by corner `c_i` with orientation `o_i`. 
 * An orientation value `o_i` indicates the corner is rotated `o_i` times 
 * clockwise from its standard orientation.
 */
const CornerCubie CornerCubieMove[6] = {
    {{UBR,0},{URF,0},{UFL,0},{ULB,0},{DFR,0},{DLF,0},{DBL,0},{DRB,0}},   
    {{DFR,2},{UFL,0},{ULB,0},{URF,1},{DRB,1},{DLF,0},{DBL,0},{UBR,2}},   
    {{UFL,1},{DLF,2},{ULB,0},{UBR,0},{URF,2},{DFR,1},{DBL,0},{DRB,0}},   
    {{URF,0},{UFL,0},{ULB,0},{UBR,0},{DLF,0},{DBL,0},{DRB,0},{DFR,0}},   
    {{URF,0},{ULB,1},{DBL,2},{UBR,0},{DFR,0},{UFL,2},{DLF,1},{DRB,0}},   
    {{URF,0},{UFL,0},{UBR,1},{DRB,2},{DFR,0},{DLF,0},{ULB,2},{DBL,1}}
};

/*! 
 * @brief EdgeCubieMove: using notation schema of "replace-by" 
 * @remark EdgeCubieMove[m][i] = {e_i,o_i} means the `i`-th edge of identity 
 * cube is replaced by edge `c_i` with orientation `o_i`. 
 * An orientation value `o_i` indicates the edge is rotated clockwise (or 
 * flipped) `o_i` times  from its standard orientation. 
 */
const EdgeCubie EdgeCubieMove[6] = { 
    {{UB,0},{UR,0},{UF,0},{UL,0},{DR,0},{DF,0},{DL,0},{DB,0},{FR,0},{FL,0},{BL,0},{BR,0}}, 
    {{FR,0},{UF,0},{UL,0},{UB,0},{BR,0},{DF,0},{DL,0},{DB,0},{DR,0},{FL,0},{BL,0},{UR,0}}, 
    {{UR,0},{FL,1},{UL,0},{UB,0},{DR,0},{FR,1},{DL,0},{DB,0},{UF,1},{DF,1},{BL,0},{BR,0}}, 
    {{UR,0},{UF,0},{UL,0},{UB,0},{DF,0},{DL,0},{DB,0},{DR,0},{FR,0},{FL,0},{BL,0},{BR,0}}, 
    {{UR,0},{UF,0},{BL,0},{UB,0},{DR,0},{DF,0},{FL,0},{DB,0},{FR,0},{UL,0},{DL,0},{BR,0}}, 
    {{UR,0},{UF,0},{UL,0},{BR,1},{DR,0},{DF,0},{DL,0},{BL,1},{FR,0},{FL,0},{UB,1},{DB,1}} 
};

const CornerCubie CornerCubieSym[4] = {
    {{URF,1},{DFR,2},{DLF,1},{UFL,2},{UBR,2},{DRB,1},{DBL,2},{ULB,1}},    
    {{DLF,0},{DFR,0},{DRB,0},{DBL,0},{UFL,0},{URF,0},{UBR,0},{ULB,0}},    
    {{UBR,0},{URF,0},{UFL,0},{ULB,0},{DRB,0},{DFR,0},{DLF,0},{DBL,0}},   
    {{UFL,3},{URF,3},{UBR,3},{ULB,3},{DLF,3},{DFR,3},{DRB,3},{DBL,3}}
}; 

const EdgeCubie EdgeCubieSym[4] = { 
    {{UF,1},{FR,0},{DF,1},{FL,0},{UB,1},{BR,0},{DB,1},{BL,0},{UR,1},{DR,1},{DL,1},{UL,1}},
    {{DL,0},{DF,0},{DR,0},{DB,0},{UL,0},{UF,0},{UR,0},{UB,0},{FL,0},{FR,0},{BR,0},{BL,0}},
    {{UB,0},{UR,0},{UF,0},{UL,0},{DB,0},{DR,0},{DF,0},{DL,0},{BR,1},{FR,1},{FL,1},{BL,1}},
    {{UL,0},{UF,0},{UR,0},{UB,0},{DL,0},{DF,0},{DR,0},{DB,0},{FL,0},{FR,0},{BR,0},{BL,0}} 
};

enum Constant {
    GN_HTM      = 20,       // the God's number in HTM
    GN_QTM      = 26,       // the God's number in QTM
    N_MOVE      = 18,       // 3*6, turn move
    N_TWIST     = 2187,     // 3^7, corner twist
    N_FLIP      = 2048,     // 2^11, edge flip
    N_SLICE     = 495,      // C(12,4), 4 ud-slices in correct locations, order omitted
    N_CORNER    = 40320,    // 8!, corners permutation
    N_EDGE8     = 40320,    // 8!, ud-edges permutation
    N_EDGE4     = 24,       // 4!, the order of 4 ud-slices
    N_SYM       = 48,       // cube symmetries
    N_SYM_D4h   = 16,       // D4h group (16 symmetries which preserve UD axis) 
    EQ_FLIPSLICE= 64430,    // [(flip,slice):D4h], equivalent class of flipslice
    EQ_CORNER   = 2768,     // [(corner):D4h], equivalent class of corner
};