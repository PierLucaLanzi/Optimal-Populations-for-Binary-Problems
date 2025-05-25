def multiplexer(binary_input):

    binary_integers = [int(x) for x in binary_input]

    address_size = {3:1,6:2,11:3,20:4,37:5}

    n = len(binary_input)

    if not (n in address_size.keys()):
        raise Exception("Multiplexer function defined only for 6, 11, and 20 bits")

    relevant_bits = address_size[n]
    weight = [2**x for x in reversed(range(relevant_bits))]

    address = 0
    for bit in reversed(range(relevant_bits)):
        address = relevant_bits+sum([x*y for x,y in zip(binary_integers[:relevant_bits], weight)])
        return binary_integers[address]
    
def majority_on_action(binary_input):
    binary_integers = [int(x) for x in binary_input]
    if (sum(binary_integers)>len(binary_integers)//2):
        return 1
    else:
        return 0

def carry(binary_input):
    if len(binary_input)%2!=0:
        raise Exception("Carry function needs a string with an even number of symbols")

    no_bits = len(binary_input)//2
    
    binary_integers = [int(x) for x in binary_input]
    
    first_addendum = binary_integers[:no_bits]
    second_addendum = binary_integers[no_bits:]

    carry = 0

    for i in reversed(range(no_bits)):
        carry = (first_addendum[i]+second_addendum[i]+carry)//2

    return carry

### From the paper "Analysis of Generalization with Symbolic Condition"
def eq(binary_input, n, k):
    if (sum([int(x) for x in binary_input])==k):
        return 1
    else:
        return 0
    
### Hierarchical multiplexer from the paper
### Automated Global Structure Extraction for Effective Local Building Block Processing in XCS
### https://direct.mit.edu/evco/article-abstract/14/3/345/1247/Automated-Global-Structure-Extraction-for
def hmhierarchical_multiplexer(s: str) -> int:
    n = len(s)
    if (n % 3 != 0) or (not (n // 3) in [3, 6, 11, 20]):
        raise Exception("Hierarchical multiplexer ")
    return multiplexer(''.join([str(mp(s[i:i+3])) for i in range(0,n,3)]))