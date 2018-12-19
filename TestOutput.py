# Grading script for H2SO4 semaphore project for CS332 Operating Systems
# @Sherri Goings, last modified 12/17/2018

# Script to test output of H2SO4 program against many commonly failed test cases.
#
# Note that the H2S04 program output statements must conform to the expected format, e.g.:
#     "oxygen produced"
#     "molecule produced"
#     "sulfur exited"
# Extra output should not be a problem, the program checks for substrings
# e.g. "produced" or "oxygen" on a given line, so it doesn't matter what
# else is on that line or even extra lines (they will just be ignored)
#
# Run a single test on the command line as follows:
#        ./student_executable args | python3 TestOutput.py
# e.g.   ./a.out 4 2 8 | python3 TestOutput.py
#
# Script will output error message for first test failed and then quit without running more tests.
# No output means that the program passed all tests (this makes it easier to run many tests in TestMain).  
# Use with TestMain.py to run a set of tests.

import sys
from enum import Enum

class Types(Enum):
    """ atom types associated with number of this type required to create molecule """
    OXYGEN = 4
    HYDROGEN = 2
    SULFUR = 1
    
class TestOneRun:
    """ Class set up to test output of single student program against all test cases """
    def __init__(self):
        """ track current state of molecule production and disassembly, list tests to run """
        self.molecules = 0
        self.available = {atom:0 for atom in Types}
        self.departed = {atom:0 for atom in Types}
        self.moleculeTests = [self.exitPreMolecule, self.badExitPrevMolecule, self.lackingAtomsForMolecule]
        self.exitTests = [self.invalidExitType, self.allPreExited, self.invalidExitOrder]

    def getAtomType(self, outputLine):
        """ return atom type found in line if valid, otherwise print message and return None """
        for atom in Types:
            if atom.name.lower() in outputLine:
                return atom
        print("Something weird produced/exited: %s" % outputLine, end="")
        return None

    # ----------------------------------------------------------------------------------
    # subtests for output lines that produce molecule, return True if pass, False if error.
    # ----------------------------------------------------------------------------------
    def exitPreMolecule(self):
        """ return False if atoms have exited before first molecule formed, True otherwise """
        if self.molecules == 0 and max(self.departed.values()) > 0:
            print("these atoms exited before first molecule created: ", end="")
            print([atom.name for atom,count in self.departed.items() if count>0])
            return False
        return True

    def badExitPrevMolecule(self):
        """ return False if incorrect num atoms departed from prev molecule, True otherwise """
        if self.molecules > 0:
            # collect list of any atoms where num departed is not expected num per molecule
            departErrors = [(atom.name, count) for atom, count in self.departed.items() if self.departed[atom] != atom.value]
            if len(departErrors) > 0:
                print("too many or too few atoms exited between previous and this molecule creations.")
                print( "Exit counts:", departErrors)
                return False
        return True

    def lackingAtomsForMolecule(self):
        """ return False if not enough available atoms of each type for molecule, True otherwise """
        createErrors = [(atom.name, count) for atom, count in self.available.items() if self.available[atom] < atom.value]
        if len(createErrors) > 0:
            print("too few atoms produced to create a new molecule")
            print("Too low atom counts:", createErrors)
            return False
        return True
    # --------------------------------- end molecule subtests -----------------------------------------

    def produceTest(self, produceLine):
        """ 
        Process the production of an atom or molecule.
        @param produceLine: one line of tested program's output stating something was "produced"
        @return: integer exit code, 0 for successful test, >0 for failure.
        """
        # not using atm but set up so unique error for each test in case of future need.
        errNum = 1
        
        if "molecule" in produceLine:
            # run all tests for lines stating that a molecule was produced 
            for test in self.moleculeTests:
                if not test():
                    return errNum
                errNum += 1

            # molecule created, remove its atoms from available, wipe all previously departed
            for atom in Types:
                self.available[atom] -= atom.value
                self.departed[atom] = 0
            self.molecules += 1

        else:
            # one off test for valid atom type. if passed, increment available atoms of this type
            atom = self.getAtomType(produceLine)
            if not atom:
                return errNum
            self.available[atom] += 1
 
        # all went well, return 0 for success
        return 0

    # ----------------------------------------------------------------------------------
    # tests for output lines that state atom exited, return True if pass, False if error.
    # ----------------------------------------------------------------------------------
    def invalidExitType(self, atom):
        """ return False if atom argument is None, true if valid atom type """
        if not atom:
            return False
        return True
    
    def allPreExited(self, curAtom):
        """ return False if all of this atom type have already exited molecule, True otherwise """
        if self.departed[curAtom] == curAtom.value:
            print("No %s atoms left in molecule to exit" % curAtom.name)
            return False
        return True

    # helper functions for invalidExitOrder
    def anyGone(self, atomType):
        """ return True if at least one of this atom type has already exited, False otherwise """
        return (self.departed[atomType] > 0)
    
    def allGone(self, atomType):
        """ return True if all atoms of this type have already exited, False otherwise """
        return (self.departed[atomType] == atomType.value)
    
    def invalidExitOrder(self, curAtom):
        """ 
        returns True if this atom is exiting in correct order, False otherwise 
        exit order must be all Hydrogen atoms -> all Sulfur atoms -> all Oxygen atoms
        each atom type checks that the counts are as expected for the other 2 types.
        """
        if (curAtom.name == "HYDROGEN" and (self.anyGone(Types.OXYGEN) or self.anyGone(Types.SULFUR))):
            print("Hydrogen did not exit first")
            
        elif (curAtom.name == "SULFUR" and (self.anyGone(Types.OXYGEN) or not self.allGone(Types.HYDROGEN))):
            print("Sulfur did not exit in correct order")
            
        elif (curAtom.name == "OXYGEN" and (not self.allGone(Types.HYDROGEN) or not self.allGone(Types.SULFUR))):
            print("Oxygen did not exit last")
        else:
            return True
        
        # if any exit ordering error occured, output departed counts at that point and return error
        print("departed counts:",[(atom.name, count) for atom, count in self.departed.items()])
        return False
    # --------------------------------- end atom exit subtests -----------------------------------------
    
    def exitTest(self, exitOutputLine):
        """ 
        Process the exit of an atom 
        (molecules exit without explicit output, immediately after production) 
        atoms are required to exit in set order by type from molecule: hydrogen -> sulfur -> oxygen
        @param exitOutputLine: one line of tested program's output stating something "exited"
        @return: integer exit code, 0 for successful test, >0 for failure.
        """
        # determine which (if any) atom exited in this line
        curAtom = self.getAtomType(exitOutputLine)

        # run all test functions
        errNum = 1
        for test in self.exitTests:
            if not test(curAtom):
                return errNum
            errNum += 1

        # no errors, update exited counts, return success.
        self.departed[curAtom] += 1
        return 0
# --------------------------------- end TestOneRun class -----------------------------------------

def main():
    """ 
    run all tests on output from single project execution (expect test data from stdin)
    separate lines into 'produced' or 'exited' type statements and run each through 
    corresponding suite of tests. Quit on 1st error found as that makes most sense for
    this particular grading task.
    """
    lines = [line for line in sys.stdin]
    oneTest = TestOneRun()
    
    # We expect two types of statement: "produced" and "exited"
    for line in lines:
        if "produced" in line:
           if oneTest.produceTest(line):
               break
        elif "exited" in line:
            if oneTest.exitTest(line):
                break

if __name__=="__main__":
    main()

