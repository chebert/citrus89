# Generate an expansion for stdin, including the current file
gcc -ansi -pedantic --std=c89 -include $1 - -E > .expand_c_snippet_temp
# Echo the expanded lines
sed -n $(($(grep -n "# 1 \"<stdin>\"" .expand_c_snippet_temp | tail -1 | cut -d : -f 1)+1)),'$'p .expand_c_snippet_temp | astyle
rm .expand_c_snippet_temp
