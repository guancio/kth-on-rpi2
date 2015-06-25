

// #include <stdio.h>

#include <uclib.h>
#include <test.h>

#define SIZE 10

#undef memcpy

void test_do(test_context_t *test)
{
    int i, j;
    char buffer1[SIZE];
    char buffer2[SIZE];
    
    test_mark(test, "string functions");
    
    test_group_start(test, "sanity check");
    {
        test_less_than(test, "Sanity check: SIZE not too large\n",
                         SIZE, 0x40-1);
    }
    test_group_end(test);
    
    test_group_start(test, "memcpy");
    {
        for(i = 0; i < SIZE; i++) {
            /* fill buffer with known data: */
            for(j = 0; j < SIZE; j++) {
                buffer1[j] = j | 0x80;
                buffer2[j] = j | 0x40;
            }
            
            /* do a copy */
            memcpy(buffer1, buffer2, i);
            
            /* now first i bytes of buffer1 should contain data from buffe 2 */
            for(j = 0; j < SIZE; j++) {
                if(j < i)
                    test_equal(test, "memcpy got good data", buffer1[j], buffer2[j]);
                else                    
                    test_equal(test, "memcpy didn't touch other data", buffer1[j], (char)(j | 0x80));
            }                                        
        }       
    }
    test_group_end(test);    
    
    
    test_group_start(test, "memset");
    {
        for(i = 0; i < SIZE / 2; i++) {
            
            /* fill it with known data */
            for(j = 0; j < SIZE; j++) buffer1[j] = 0x66;
            
            /* set 0...i-1 bytes with i */
            memset(buffer1, i, i);
            
            /* check it ! */
            for(j = 0; j < SIZE; j++) {
                if(j < i)
                    test_equal(test, "memset set", i, buffer1[j]);
                else            
                    test_equal(test, "memset untouched", 0x66, buffer1[j]);
            }
        }
    }            
    test_group_end(test);
    
    
    test_group_start(test, "memcmp");
    {
        test_equal( test, "memcmp (1)", memcmp("ABCD", "ABCE", 0), 0);
        test_equal( test, "memcmp (2)", memcmp("ABCD", "ABCE", 1), 0);
        test_equal( test, "memcmp (3)", memcmp("ABCD", "ABCE", 2), 0);
        test_equal( test, "memcmp (4)", memcmp("ABCD", "ABCE", 3), 0);
        test_less_than( test, "memcmp (5)", memcmp("ABCD", "ABCE", 4), 0);
        test_larger_than( test, "memcmp (6)", memcmp("ABCD", "ABCC", 4), 0);
    }
    test_group_end(test);    
    
  
    test_group_start(test, "strlen");
    {
        test_equal(test, "strlen 1", 0, strlen(""));
        test_equal(test, "strlen 2", 1, strlen("A"));
        test_equal(test, "strlen 3", 7, strlen("ABCDEFG"));       
    }            
   test_group_end(test);    
    
    
    test_group_start(test, "strcpy");
    {
        for(i = 0; i < SIZE; i++) {  
            
            /* fill buffer with known data: */
            for(j = 0; j < SIZE; j++)
                buffer2[j] = 0xFF;
            
            for(j = 0; j < i; j++) 
                buffer1[j] = 'A' + j;
            buffer1[i] = '\0';
            
            
            /* do a copy */
            strcpy(buffer2, buffer1);
            
            /* now first i bytes of buffer1 should contain data from buffe 2 */            
            for(j = 0; j < SIZE; j++) {
                if(j <= i)
                    test_equal(test, "strcpy got good data", buffer1[j], buffer2[j]);
                else                    
                    test_equal(test, "strcpy didn't touch other data", buffer2[j], (char)0xFF);
            }                                        
        }       
    }
    test_group_end(test);    
    
    
      
    test_group_start(test, "strncpy");
    {
        int i;
        char buffer[32 + 1];
        
        /* pad with zero */
        strncpy(buffer, "ABC", 32);
        for(i = 0; i < 32; i++) {
            test_equal(test, "strcpy (1)", buffer[i],
                       i < 3 ? "ABC"[i] : '\0');
        }
        
        /* don't copy more than request */
        memset(buffer, '?', 32);
        strncpy(buffer, "ABCDE", 4);
        for(i = 0; i < 5; i++) {
            test_equal(test, "strcpy (2)", buffer[i], 
                       i < 4 ? "ABCD"[i] : '?'); /* last one is an untouched  */
        }                
    }
    test_group_end(test);    
    
    test_group_start(test, "strcmp");
    {        
        test_equal( test, "strcmp(, )", 0 , strcmp("", ""));
        test_not_equal( test, "strcmp(A, )", 0 , strcmp("A", ""));
        test_not_equal( test, "strcmp(, A)", 0 , strcmp("", "A"));
        
        test_equal( test, "strcmp(ABC, ABC)", 0 , strcmp("ABC", "ABC"));
        test_not_equal( test, "strcmp(ABCD, ABC)", 0 , strcmp("ABCD", "ABC"));
        test_not_equal( test, "strcmp(ABC, ABCD)", 0 , strcmp("ABC", "ABCD"));
    }
    test_group_end(test);    
    
}
