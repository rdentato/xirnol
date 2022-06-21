# xirnol
An implementation of the [`knight` programming language](https://github.com/knight-lang/knight-lang).

WHile still usable as a general purpose `knight` interpreter, its purpose is to serve as an example for the `skp` (text matching/parsing) and `val` (NaNboxed values) libraries which you can find in the `libs` directory.

It currently lacks support for the `EVAL` function and there is no plan to add it.

Instead there are the following extensions:

   - Floating point are supported.
  
   - The function `$` converts a double to integer

   - `VALUE string` considers string as the name of a variable and returns its value (or null if there is no such a variable)
  
   - `=` also accepts a string and consider it to be a variable name.

Example:
   `';= + "x" "0" 10 ; O x0 : O V + "x" "0"'`

   will print:

   ```
   10
   10
   ```
Example:
   `';O / 3 2 ; O / 3 2.0 : O / 3.0 2'`
   
   will print:

   ```
   1
   1
   1.500000
   ```

The name is a lojban cmene (proper noun) related to the word *xirno'i* which means "knighted noble" (as opposed to *xirsoi* which means cavalryman).
