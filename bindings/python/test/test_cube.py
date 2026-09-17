from pycube import solve, facecube, permutation, solvable, CUBE_ID, SolveError
import pytest 

ms = "U F U' L2 R L' D2 B"
fc = facecube(ms)

@pytest.mark.fast
def test_solvability(): 
    assert not solvable("U D F")
    assert solvable(fc)


@pytest.mark.fast
def test_permutation(): 
    cs0 = permutation(fc,0)
    assert len(cs0) == 108
    cs1 = permutation(ms,1)
    assert len(cs1) == 40
    cs2 = permutation(ms)
    assert cs2 != ""
    

@pytest.mark.slow
@pytest.mark.solver
def test_solve(): 
    try: 
        s = solve(fc)
        assert facecube(s,fc) == CUBE_ID
    except Exception as e:
        assert type(e) == SolveError
