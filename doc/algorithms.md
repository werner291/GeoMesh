# Algorithms

WARNING: LOGIC AHEAD!

In this file, several algorithms used in GeoMesh will be defined and analysed.

All arrays are 0-indexed.

## Find XOR-closest binary search

### Problem

Given a set of *n* 128-bit integers, find the address that is closest to a given query integer Q.

### Requirement

Many queries will be made, on average the algorithm must do much better than linear time.

Sorting the set is allowed once only (Not once per query), and does not count towards the maximum O(log n) time. 

### Algorithm

Notation:
 * For some k-bit integer a, the [] operater designates "take the bit at that position from the left". 
   For example, for k = 8 and a = 01000000 (in binary), we get a[1] = 1.
   
 * = is the assignment operator, == is the comparison operator.
 
 * For some array A, the notation A[a,b] denotes the subarray of A from indices *a* to *b* inclusive.
 
 * When discussing a loop, for some variable *x*, *x_old* refers to the value of *x* at the start of
   the iteration, while *x_new* referes to the value of *x* at the start of the next iteration.

Once:
    Create a sorted array A from the set of integers.

Each query, for some input query address Q:

    int a = 0;
    int b = A.length - 1;

    int match = 0; // The bit position to test to check for prefix equality

    while (a < b) {
        
        if (A[a][match] == 0 && A[b][match] == 1) {
            // Find split point through binary search
            int c = a;
            int d = b;
                
            while (c < d + 1) {
                int middle = c + (d-c)/2; // Take average, prevent overflow.
                
                if (A[middle][match] == 0) {
                    c = middle;
                } else { // Must be 1 by excluded middle
                    d = middle;
                }
            }
            
            if (Q[match] == 0) {
                b = c;
            } else {
                a = d;
            }
            
            match += 1;
        }
    
    }
    
    return A[a];

### Correctness proof

We will first prove the correctness of the inner lopp before moving to the outer loop.

#### Inner loop    

##### Precondition

A[a][match] == 0 && A[b][match] == 1 && a < b

##### Postcondition (to prove)

1. A[c][match] == 0 && A[d][match] == 1
2. d = c + 1
3. a <= c,d <= b.

##### Loop invariant

At the start of the iteration, A[c][match] == 0 && A[d][match] == 1, c < d

Initialisation:
    
    Since a == c and b == d:
        c < d is true since a < b.
        A[c][match] == 0 && A[d][match] == 1 is true since A[a][match] == 0 && A[b][match] == 1
        a <= c,d <= b is trivially true
    
Maintenance:
    
    The integer *midddle* is initialized as the average of a and b. Thus, we know a <= middle <= b.
    
    Case A[middle][match] == 0:
        c is changed to the value of *middle*.
        A[c_new][match] == 0 is still true since c_new == middle and A[middle][match] == 0.
        d is unmodified, so A[d][match] == 1 is still true.
        The loop guard guarantees that c_old + 1 < d. Therefore:
            c_new = middle = floor( (c_old + d) / 2 ) 
                < floor( ((d-1) + d) / 2 ) 
                = floor( (2*d-1) / 2 )
                = floor( d-1/2 ) {d is an integer}
                = d-1
            Since d is not modified in this case, d_old == d_new, thus c_new < d_new - 1. c < d is still true.
        Also, c_new is assigned as the average of c_old and d_old, and c_old and d_old,
        thus c_old <= c_new <= d_old, thus a <= c_new <= b.
        The loop invariant is still true.
    Case A[middle][match] == 1:
        d is changed to the value of *middle*. 
        A[d_new][match] == 1 is true since A[middle][match] == 1 and d_new == middle.
        The loop guard guarantees that c_old + 1 < d. Therefore:
            d_new = floor( (c + d_old)/2)
                  > floor( (c + c + 1)/2 )
                  = floor( c + 1/2 ) { c is an integer}
                  = c
            Since c is not modified, c_new = c, thus d_new > c_new. c < d is still true.
        Also, d_new is assigned as the average of c_old and d_old, and c_old and d_old,
        thus c_old <= d_new <= d_old, thus a <= d_new <= b.
        The invariant is still true.
     
     A[middle][match] is a bit, there are no other cases. In both cases, the invariant remains true, so it is true
     at the end of the iteration.
     
Termination:
    c < d + 1 is now false, so we get c >= d + 1. The LI says c < d. Thus, c = d + 1. The LI also guarantees that
    A[c][match] == 0 && A[d][match] == 1. Thus, the postcondition is true.

#### Outer loop

Loop invariant: 
    1. The element of A closest in terms of xor-distance to Q is between positions *a* and *b*, both inclusive.
    2. The *match* first bits of all elements in A[a,b] are the same.
    3. For all elements in A[a,b] holds that, for any consecutive elements *x* and *y*, *not (x[match] == 1 && y[match] == 0)*.

Initialisation:
    In this case, A == A[a,b]:
    1. The closest element of A to Q is in A. Since A == A[a,b], that element must also be in A[a,b].
    2. *match == 0*, so the second point is true since the 0 first bits of the elements are all an empty string of bits.
    3. A is sorted. If there was a pair of consecutive elements for which x[0] == 1 && y[0] == 0, this would mean that
       x > y, which would be a violation of A being sorted. The third point is thus true as well.
    
Maintenance:

    Case A[a][match] == 0 && A[b][match] == 1:
    
        a < b is true by the guard of the while loop, and A[a][match] == 0 && A[b][match] == 1 by the if-statement guard.
        Thus, the precondition for the inner loop is true. The inner loop is then executed, which means the postcondition
        of the inner loop is now true: A[c][match] == 0 && A[d][match] == 1, c = d + 1, a <= c,d <= b.
    
        The LI states the first *match* bits are the same in all elements in A[a,b]. Thus, for any pair of elements
        *x* and *y* in A[a,b], x[0] == 1 && y[0] == 0 would imply that x > y. Since A is sorted, so is A[a,b], and
        x[match] == 1 && y[match] == 0 will never be true for consecutive pairs (x,y) in A[a,b].
        
        Since A[c] == 0, we know that for all elements *x* in A[a,c], we have *x[match] == 0*, and since A[d] == 1,
        we know that for all elements *y* in A[d,b], we have *y[match] == 1*. Since c = d+1, c and d are consecutive
        indices in A[a,b]. Thus, the subarrays A[a,c] and A[d,b] are a partitioning of A[a,b], leaving out no elements.
        
        Case Q[match] == 0:
            b is reassigned to c, thus A[a,b] == A[a,c].
            
            We already know the element closest to Q is in A[a_old,b_old], so it must either be in A[a,c] or A[d,b].
            
            Let *x* be an element of A[a,c], and *y* an element of A[d,b]. The first *match* bits are the same
            in *x* and *y*, which means that the bit at index *match* is the most significant different between
            *x* and *y*. This implies that *x XOR Q < y XOR Q*. Thus, all elements of A[a,c] have an XOR distance
            to Q smaller than any element in A[d,b]. Thus, the element of A closest to Q is in A[a,c], thus also
            in A[a,b_new].
            
            After *match* is incremented, the first *match_new* bits of all elements in A[a,b] will be the same since
            the invariant says the first *match_old* bits are the same in all elements of A[a_old,b_old], and all
            *match_old*th bits in the elements of A[a,c] are 0. Part 2 of the invariant is satisfied.
            
            The third part of the invariant is true since A was sorted, and all *match_new* first bits of all elements
            int A[a,c] are the same.
            
            Since all 3 parts of the invariant are true, the invariant iself is true.
            
        Case Q[match] == 1:
            This case is not proven here, but it is symmetrical to the previous one.
    
    Otherwise:
        *a* and *b* are not modified, so the loop invariant must still be true.

Termination:
    
    The loop guard became false: not (a < b) == (a >= b).
    Since the postcondition of the inner loop guarantees that *c* and *d* are never outside the range [a,b],
    and these are the only values assigned to *a* and *b*, we derive that a == b.
    
    Thus, the subarray A[a,b] only has one element. The loop invariant guarantees that the element closest to Q
    is in A[a,b]. The element we're looking for must be the single element in that 1-element subarray.
    
    This value is returned, thus the algorithm correctly solves the problem.
    
### Running time analysis (informal)

In this analysis, we will assume the distribution of numbers is uniform in the given set.

There is an outer loop that iterates until a == b. The difference between a and b is roughly
halved each iteration, so this loop iterates O(log n) times.

The loop body is dominated by an inner loop which performs a binary search on a range of size b - a.
The body of the inner loop takes constant time.

    T(n) = O(log(n) + log(n/2) + log(n/4) + log(n/8) + ...)
         = O((log(n))^2)
         
This could perhaps be improved by using a rooted binary tree to store the integers, but the increase in
efficiency will be questionable since using an array will reduce the number of cache misses. To be tested.