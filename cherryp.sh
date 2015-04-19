git rev-list --reverse --topo-order 68321901053b634f27adb96c6323749e66d3db5d^..33c028ab962471762c7cc69d41f75e5d1a2ec58a | while read rev 
do 
  git cherry-pick $rev || break 
done 
