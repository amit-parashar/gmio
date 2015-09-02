#include "../../src/gmio_stl/stl_io.h"
#include "../../src/gmio_support/stl_occ.h"

#include <QtCore/QFile>
#include "../../src/gmio_support/stream_qt.h"

int main()
{
    // OpenCascade
    Handle_StlMesh_Mesh stlMesh;
    gmio_stl_occmesh_creator(stlMesh);

    // Qt
    QFile file;
    gmio_stream_qiodevice(&file);

    return 0;
}
