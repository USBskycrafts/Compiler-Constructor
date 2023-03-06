void main() {
  int64[] arr
  int64 n

  n <- input()
  arr <- new Array(n)
  int64 i
  while(i < n) :body :exit
  {
:body
    int64 n
    n <- input()
    arr[i] <- n
    i <- i + 1
    continue
  }
:exit
  i <- 0
  while(i < n) :body2 :exit2
  {
  :body2
    int64 j
    int64 m
    m <- n - 1
    m <- m - i
    while(j < m) :body3 :exit3
    {
    :body3
      int64 temp
      temp <- j + 1
      int64 former
      int64 latter 
      former <- arr[j]
      latter <- arr[temp]
      if(former > latter) :true :false
    :true
        int64 c, d
        c <- arr[temp]
        d <- arr[j]
        arr[temp] <- d
        arr[j] <- c
    :false
    j <- j + 1
    continue
    }
  :exit3
    i <- i + 1
    continue
  }

  :exit2
  print(arr)
}