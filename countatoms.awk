#! /usr/bin/awk -f

/^feld/ {
    sub(/^feld_.*=/, "", $0);
    for(i = 1; i <= length($0); ++i) {
	l = substr($0, i, 1);
	if (l ~ /[0-9a-z]/)
	    ++counts[l];
    }
}

END {
    once = twice = multi = 0;

    for (x in counts)
	if (counts[x] == 1)
	    ++once;
	else if (counts[x] == 2)
	    twice += 2;
	else multi += counts[x];
    
    print once, twice, multi;
}
