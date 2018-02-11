for i in "$@"
do
git grep $i src/*  | grep -v src/external/
done
