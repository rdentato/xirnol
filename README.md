# xirnol
An implementation of the [`knight` programming language](https://github.com/knight-lang/knight-lang).

WHile still usable as a general purpose `knight` interpreter, its purpose is to serve as an example for the `skp` (text matching/parsing) and `val` (NaNboxed values) libraries which you can find in the `libs` directory.

It currently lacks support for the `EVAL` function and there is no plan to add it.

Instead there are the following extensions:

   - Floating point are supported. The expressions `/ 3 2` will give `1` but `/ 3.0 2` will give `1.5`
   
   - To convert a double to an intger just sum `0`:  `+ 2.3 0` will give `2`.

   - `VALUE string` considers string as the name of a variable and returns its value (or null if there is no such a variable)
  
   - `=` also accepts a an expression returing a string and consider the result to be the name of a variable (which could even not exist before).
  
   - Variables can hold stacks: `X_PUSH my_stack 3` will push `3` in the `my_stack` variable. `X_POP my_stack` pops the top value and returns it. `LEN`, if applied to stack, will return the number of elements in the stack.

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
