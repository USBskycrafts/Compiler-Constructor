void main () {
    int64[] number
    int64 tempNumber, retVal

    number <- new Array(7)
    tempNumber <- length number 1

    retVal <- op1 (1)
    number[0] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op1 (2)
    number[1] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op2 (3)
    number[2] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op3 (4)
    number[3] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op4 (5)
    number[4] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op5 (6)
    number[5] <- retVal
    tempNumber <- tempNumber + 8

    retVal <- op6 (7)
    number[6] <- retVal
    tempNumber <- tempNumber + 8

    printArr (number)
    return
}

int64 op1 (int64 number) {
    int64[] temp_spot, num_array, arr
    int64 arr_elem, number2

    temp_spot <- new Array(3)
    temp_spot[0] <- number

    number <- number < 1
    num_array <- new Array(5)
    arr <- new Array(3)
    arr_elem <- arr[0]

    number <- temp_spot[0]
    return number
}

int64 op2 (int64 number) {

    int64[] temp_spot, num_array
    int64 number2

    temp_spot <- new Array(3)
    temp_spot[0] <- number
    number <- number < 1
    num_array <- new Array(5)
    number <- temp_spot[0]
    return number
}

int64 op3 (int64 number) {

    int64[] temp_spot, num_array
    int64 number2

    temp_spot <- new Array(3)
    temp_spot[0] <- number
    number <- number < 1
    num_array <- new Array(5)
    number <- number * 200000
    number <- temp_spot[0]
    return number
}

int64 op4 (int64 number) {

    int64[] temp_spot, num_array
    int64[][][] threeD
    int64 number2, first_dimension_length, second_dimension_length, third_dimension_length

    temp_spot <- new Array(3)
    temp_spot[0] <- number
    number <- number < 1
    num_array <- new Array(5)
    number <- temp_spot[0]
    threeD <- new Array(6,6,6)

    first_dimension_length <- length threeD 1
    second_dimension_length <- length threeD 2
    third_dimension_length <- length threeD 3

    return number
}

int64 op5 (int64 number) {
    return number
}

int64 op6 (int64 number) {

    int64[] temp_spot, num_array

    temp_spot <- new Array(3)
    temp_spot[0] <- number
    number <- number < 1
    num_array <- new Array(5)
    number <- temp_spot[0]
    return number
}

void printArr (int64[] num_array) {

    int64 comp, count, number

    count <- 0

    while (count < 7) :top :end

    :top
        number <- num_array[count]
        print (number)

        count <- count + 1
        continue
    :end

    return
}