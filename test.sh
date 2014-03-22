#!/bin/sh

export NETRC_FILE=`mktemp`
cat <<EOF >$NETRC_FILE
machine imap.gmail.com login login1@gmail.com password password1 port 993

machine api.heroku.com
login account2@gmail.com
password a001d820-b09d-11e3-88fe-425861b86ab6

machine smtp.gmail.com login account1@gmail.com password password2 port 587

machine code.heroku.com
login login2@gmail.com
password a001db0e-b09d-11e3-88fe-425861b86ab6
EOF

fail_num=0

function test_case {
    local actual=`$1`
    test "$3" == :ignore-port &&
    actual=`echo "$actual" | sed '/^Port:/d'`
    local expected=`echo "$2" | sed "s|\\$NETRC_FILE|$NETRC_FILE|"`
    test "$actual" != "$expected" &&
    let fail_num+=1 &&
    echo "\`$1' failed" &&
    printf "Actual:\n%s\nExpected:\n%s\n\n" "$actual" "$expected"
}

read -r -d '' expected <<EOF
Machine:   imap.gmail.com
Login:     login1@gmail.com
Password:  password1
Port:      993
File Name: $NETRC_FILE
Status:    0
EOF
test_case "./netrc imap.gmail.com" "$expected"

read -r -d '' expected <<EOF
Machine:   api.heroku.com
Login:     account2@gmail.com
Password:  a001d820-b09d-11e3-88fe-425861b86ab6
File Name: $NETRC_FILE
Status:    0
EOF
test_case "./netrc api.heroku.com" "$expected" :ignore-port

read -r -d '' expected <<EOF
Machine:   smtp.gmail.com
Login:     account1@gmail.com
Password:  password2
Port:      587
File Name: $NETRC_FILE
Status:    0
EOF
test_case "./netrc smtp.gmail.com" "$expected"

read -r -d '' expected <<EOF
Machine:   code.heroku.com
Login:     login2@gmail.com
Password:  a001db0e-b09d-11e3-88fe-425861b86ab6
File Name: $NETRC_FILE
Status:    0
EOF
test_case "./netrc code.heroku.com" "$expected" :ignore-port

read -r -d '' expected <<EOF
Machine:   smtp.gmail.com
Login:     account1@gmail.com
Password:  password2
Port:      587
File Name: $NETRC_FILE
Status:    0
EOF
test_case "./netrc smtp.gmail.com account1@gmail.com" "$expected"

read -r -d '' expected <<EOF
Machine:   smtp.gmail.com
Login:     wrong@gmail.com
Password:  NULL
File Name: $NETRC_FILE
Status:    2
EOF
test_case "./netrc smtp.gmail.com wrong@gmail.com" "$expected" :ignore-port

if [ $fail_num -eq 0 ]
then
    echo "All tests passed"
else
    echo "$fail_num test case(s) failed"
fi
rm -f $NETRC_FILE
exit $fail_num
