[prog]
    [instr]
        [args2]
            [instr]
                [assign_args]
                    [instr]
                        [block]
                            [instr]
                                [string (3)] 'Hi!'
                                [func_1 (241)] 'OUT'
                        [block_ret] ''
                    [varref] 'hi'
                [assign] '='
            [instr]
                [args2]
                    [instr]
                        [variable] 'hi'
                        [call] 'CALL'
                    [instr]
                        [args2]
                            [instr]
                                [variable] 'hi'
                                [func_1 (241)] 'OUT'
                            [instr]
                                [args2]
                                    [instr]
                                        [assign_args]
                                            [instr]
                                                [args2]
                                                    [variable] 'hi'
                                                    [number] '0'
                                                [func_2 (242)] '+'
                                            [varref] 'x'
                                        [assign] '='
                                    [instr]
                                        [args2]
                                            [instr]
                                                [variable] 'x'
                                                [func_1 (241)] 'OUT'
                                            [instr]
                                                [variable] 'x'
                                                [call] 'CALL'
                                        [func_2 (242)] ';'
                                [func_2 (242)] ';'
                        [func_2 (242)] ';'
                [func_2 (242)] ';'
        [func_2 (242)] ';'
    [prog_end] ''
48 nodes
