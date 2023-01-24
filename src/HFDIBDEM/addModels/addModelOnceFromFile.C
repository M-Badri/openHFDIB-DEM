/*---------------------------------------------------------------------------*\
                        _   _ ____________ ___________    ______ ______ _    _
                       | | | ||  ___|  _  \_   _| ___ \   |  _  \|  ___| \  / |
  ___  _ __   ___ _ __ | |_| || |_  | | | | | | | |_/ /   | | | || |_  |  \/  |
 / _ \| '_ \ / _ \ '_ \|  _  ||  _| | | | | | | | ___ \---| | | ||  _| | |\/| |
| (_) | |_) |  __/ | | | | | || |   | |/ / _| |_| |_/ /---| |/ / | |___| |  | |
 \___/| .__/ \___|_| |_\_| |_/\_|   |___/  \___/\____/    |___/  |_____|_|  |_|
      | |                     H ybrid F ictitious D omain - I mmersed B oundary
      |_|                                        and D iscrete E lement M ethod
-------------------------------------------------------------------------------
License

    openHFDIB-DEM is licensed under the GNU LESSER GENERAL PUBLIC LICENSE (LGPL).

    Everyone is permitted to copy and distribute verbatim copies of this license
    document, but changing it is not allowed.

    This version of the GNU Lesser General Public License incorporates the terms
    and conditions of version 3 of the GNU General Public License, supplemented
    by the additional permissions listed below.

    You should have received a copy of the GNU Lesser General Public License
    along with openHFDIB. If not, see <http://www.gnu.org/licenses/lgpl.html>.

InNamspace
    Foam

Contributors
    Martin Isoz (2019-*), Martin Kotouč Šourek (2019-*),
    Ondřej Studeník (2020-*)
\*---------------------------------------------------------------------------*/
#include "addModelOnceFromFile.H"

using namespace Foam;

//---------------------------------------------------------------------------//
addModelOnceFromFile::addModelOnceFromFile
(
    const dictionary& addModelDict,
    const Foam::fvMesh& mesh,
    const bool startTime0,
    geomModel* bodyGeomModel,
    List<labelList>& cellPoints
)
:
addModel(mesh, bodyGeomModel, cellPoints),
addModelDict_(addModelDict),
addMode_(word(addModelDict_.lookup("addModel"))),
coeffsDict_(addModelDict_.subDict(addMode_+"Coeffs")),
bodyAdded_(false),
fileName_("constant/" + (word(coeffsDict_.lookup("fileName")))),
ifStream_(fileName_.toAbsolute())
{
    if(!ifStream_.opened())
    {
        FatalErrorIn("addModelOnceFromFile::addModelOnceFromFile()") 
        << "Can not open IFstream for file: " 
        << fileName_.toAbsolute() << nl << exit(FatalError);
    }

    if(!startTime0)
    {
        ifStream_.setEof();
    }
}

addModelOnceFromFile::~addModelOnceFromFile()
{
}
//---------------------------------------------------------------------------//

geomModel* addModelOnceFromFile::addBody
(
    const volScalarField& body,
    PtrList<immersedBody>& immersedBodies  
)
{
    string line;
    ifStream_.getLine(line);
    if(line == "")
    {
        InfoH << addModel_Info << "-- addModelMessage-- "
            << "Skipping empty line at " << ifStream_.lineNumber() - 1 <<endl;
        bodyAdded_ = false;
        return geomModel_().getGeomModel();
    }
    IStringStream stringStream(line);
    vector position(stringStream);

    geomModel_->bodyMovePoints(position - geomModel_->getCoM());

    volScalarField helpBodyField_ = body;
    geomModel_->createImmersedBody(
        helpBodyField_,
        octreeField_,
        cellPoints_
    );

    bool canAddBodyI = !isBodyInContact(immersedBodies);

    reduce(canAddBodyI, andOp<bool>());

    bodyAdded_ = true;
    return geomModel_().getGeomModel();
}