# Handling interop between async JavaScript and C compiled to WebAssembly

# The problem
Let's say you want to export a function from JS to wasm to do some i/o with fetch:
```
async function fetch_export(path) {
  ...
  let result = await fetch(path)
  ...
  return data_from_fetch;
}
```
It doesn't work. After stepping through with a debugger you will verify that the JS function is returning the value you expect, but your C function is receiving zero.
What gives?
Long story short, async/await is some magic, and what actually happens as far as C is concerned, is that the imported function returns as soon as it hits `await`, with the return value being garbage.
This could be resolved by running fetch synchronously, but since that isn't allowed, we are stuck.

# The solution
Asynchronous code in JS works by hooking up callbacks to promises, so when in Rome, do as the Romans do.
We'll have to manually transform our C code to work in the same way.

Is this painful? Yes.
Will you have to rewrite your entire program because a single async call "infects" the whole thing? Yes.
But we did get it working.

# Build
Run build.bat, tested on Windows, probably works on other platforms. I've also included the built binary in the repo (demo.wasm) if you don't want or have clang set up.

# Run
Unfortunately a web server is required to run this demo, anything that can serve files in the repo directory will do.
I've found python to be a simple zero-setup solution that you are likely to already have present on your system. You can serve the current working directory on port 8000 with this: `python -m http.server`
and then navigate your web browser to http://localhost:8000/demo.html