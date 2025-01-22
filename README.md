In this challenge, I build a DNS server that's capable of parsing and
creating DNS packets, responding to DNS queries, handling various record types
and doing recursive resolve. Along the way we'll learn about the DNS protocol,
DNS packet format, root servers, authoritative servers, forwarding servers,
various record types (A, AAAA, CNAME, etc) and more.

# Passing the first stage

The entry point for your `your_program.sh` implementation is in
`src/server.cpp`. Study and uncomment the relevant code, and push your changes
to pass the first stage:

```sh
git commit -am "pass 1st stage" # any msg
git push origin master
```

# Stage 2 & beyond

1. Ensure you have `cmake` installed locally
1. Run `./your_program.sh` to run your program, which is implemented in
   `src/server.cpp`.
1. Commit your changes and run `git push origin main` to submit your solution
   to CodeCrafters. Test output will be streamed to your terminal.
