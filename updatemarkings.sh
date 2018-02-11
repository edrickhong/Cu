git grep TODO src/*  | grep -v src/external/ > TODO.txt
git grep FIXME src/* | grep -v src/external/ > FIXME.txt
git grep MARK src/*  | grep -v src/external/ > MARK.txt
git grep NOTE src/*  | grep -v src/external/ > NOTE.txt
