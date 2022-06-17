[prog]
    [instr]
        [args2]
            [instr]
                [assign_args]
                    [string (3)] 'abc'
                    [varref] 'x'
                [assign] '='
            [instr]
                [args2]
                    [instr]
                        [assign_args]
                            [instr]
                                [args4]
                                    [variable] 'x'
                                    [number] '0'
                                    [number] '1'
                                    [string (1)] '2'
                                [func_4 (244)] 'SET'
                            [varref] 'y'
                        [assign] '='
                    [instr]
                        [args2]
                            [instr]
                                [assign_args]
                                    [instr]
                                        [args4]
                                            [variable] 'x'
                                            [number] '1'
                                            [number] '1'
                                            [number] '3'
                                        [func_4 (244)] 'SET'
                                    [varref] 'z'
                                [assign] '='
                            [instr]
                                [args2]
                                    [instr]
                                        [variable] 'x'
                                        [func_1 (241)] 'OUT'
                                    [instr]
                                        [args2]
                                            [instr]
                                                [variable] 'y'
                                                [func_1 (241)] 'OUT'
                                            [instr]
                                                [variable] 'z'
                                                [func_1 (241)] 'OUT'
                                        [func_2 (242)] ';'
                                [func_2 (242)] ';'
                        [func_2 (242)] ';'
                [func_2 (242)] ';'
        [func_2 (242)] ';'
    [prog_end] ''
53 nodes
