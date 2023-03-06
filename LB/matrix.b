void main() {
  int64 m, n
  m <- input()
  n <- input()
  int64[][] x, y, z
  x <- new Array(m, n)
  load(x)
  m <- input()
  n <- input()
  y <- new Array(m, n)
  load(y)
  m <- length x 0
  n <- length y 1
  z <- new Array(m, n)
  multiple(x, y, z)
  print(z)
}


void load(int64[][] matrix)
{
  int64 m, n
  m <- length matrix 0
  n <- length matrix 1
  int64 i
  while(i < m) :body1 :exit1
  {
  :body1
    int64 j
    while(j < n) :body2 :exit2
    {
    :body2
      int64 temp
      temp <- input()
      matrix[i][j] <- temp
      j <- j + 1
      continue
    }
    :exit2
    i <- i + 1
    continue
  }
  :exit1
}

void multiple(int64[][] x, int64[][] y, int64[][] z)
{
  int64 m, n, l
  m <- length x 0
  n <- length y 1
  l <- length x 1
  int64 i
  while(i < m) :body1 :exit1
  {
  :body1
    int64 j
    while(j < n) :body2 :exit2
    {
    :body2
      int64 k, sum
      while(k < l) :body3 :exit3
      {
      :body3
        int64 a, b, temp
        a <- x[i][k]
        b <- y[k][j]
        temp <- a * b
        sum <- sum + temp
        k <- k + 1
        continue
      }
      :exit3
      z[i][j] <- sum
      j <- j + 1
      continue
    }
    :exit2
    i <- i + 1
    continue
  }
  :exit1
}