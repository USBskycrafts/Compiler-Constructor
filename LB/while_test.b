void main() {
  int64 n, i, sum
  n <- input()
  {
    :body
    int64 temp
    temp <- input()
    sum <- sum + temp
    i <- i + 1
    continue
  } while(i < n) :body :exit
  :exit
  print(sum)
}