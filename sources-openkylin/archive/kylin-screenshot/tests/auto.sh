#!/bin/bash
qmake
make
#./unit_test_default/unit_test_default
#rm moc_*
#lcov -d . -c -o r.info
#lcov -remove r.info '/usr/*' --output-file coverage.info
#genhtml coverage.info -o result
