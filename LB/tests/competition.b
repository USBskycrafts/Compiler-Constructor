
void main ()
{
  int64 dim
  dim <- input()
  print(dim)
  int64[][] m3
  int64[][] m2
  int64[][] m1
  m1 <- new Array (dim, dim)
  m2 <- new Array (dim, dim)
  m3 <- new Array (dim, dim)
  computeAndPrint(m1, m2, m3)
}





void computeAndPrint (int64[][] m1, int64[][] m2, int64[][] m3)
{
  initMatrix(m1)
  initMatrix(m2)
  matrixOperation(m1, m2, m3)
  matrixOperation(m3, m2, m1)
  matrixOperation(m3, m1, m2)
  matrixOperation(m1, m2, m3)
  matrixOperation(m3, m2, m1)
  matrixOperation(m3, m1, m2)
  int64 t
  t <- totalSum(m1)
  print(t)
  t <- totalSum(m2)
  print(t)
  t <- totalSum(m3)
  print(t)
}





void initMatrix (int64[][] m)
{
  int64 index1
  int64 l2
  int64 l1
  l1 <- length m 0
  l2 <- length m 1
  while (index1 < l1) :new_label0 :new_label11 
  :new_label0
  {
  int64 index2
  while (index2 < l2) :new_label1 :new_label10 
  :new_label1
  {
  int64 valueToStore
  valueToStore <- input()
  int64 keepNormalize
  int64 r
  int64 rDefault
  int64 lowThreshold
  int64 highThreshold
  rDefault <- 400
  lowThreshold <- 100
  highThreshold <- 90
  r <- rDefault
  keepNormalize <- 1000
  while (keepNormalize >= 0) :new_label2 :new_label9 
  :new_label2
  {
  keepNormalize <- keepNormalize - 1
  if (valueToStore >= highThreshold) :new_label3 :new_label4 
  :new_label3
  {
  valueToStore <- valueToStore - r
  r <- r - 10
}


  goto :new_label5
  :new_label4
  {
  r <- r + 42
}


  :new_label5
  if (r < lowThreshold) :new_label6 :new_label7 
  :new_label6
  {
  r <- rDefault
}


  goto :new_label8
  :new_label7
  {
}


  :new_label8
}


  continue
  :new_label9
  m[index1][index2] <- valueToStore
  index2 <- index2 + 1
}


  continue
  :new_label10
  index1 <- index1 + 1
}


  continue
  :new_label11
}





void matrixOperation (int64[][] m1, int64[][] m2, int64[][] m3)
{
  int64 m3_l2
  int64 m3_l1
  int64 m2_l2
  int64 m2_l1
  int64 m1_l2
  int64 m1_l1
  m1_l1 <- length m1 0
  m1_l2 <- length m1 1
  m2_l1 <- length m2 0
  m2_l2 <- length m2 1
  m3_l1 <- length m3 0
  m3_l2 <- length m3 1
  if (m1_l2 = m2_l1) :new_label0 :new_label1 
  :new_label0
  {
}


  goto :new_label2
  :new_label1
  {
  return 
}


  :new_label2
  if (m3_l1 = m1_l1) :new_label3 :new_label4 
  :new_label3
  {
}


  goto :new_label5
  :new_label4
  {
  return 
}


  :new_label5
  if (m3_l2 = m2_l2) :new_label6 :new_label7 
  :new_label6
  {
}


  goto :new_label8
  :new_label7
  {
  return 
}


  :new_label8
  {
  int64 j
  int64 i
  while (i < m3_l1) :new_label9 :new_label12 
  :new_label9
  {
  j <- 0
  while (j < m3_l2) :new_label10 :new_label11 
  :new_label10
  {
  m3[i][j] <- 0
  j <- j + 1
}


  continue
  :new_label11
  i <- i + 1
}


  continue
  :new_label12
}


  int64 tot
  int64 k
  int64 j
  int64 i
  while (i < m1_l1) :new_label13 :new_label24 
  :new_label13
  {
  j <- 0
  while (j < m2_l2) :new_label14 :new_label23 
  :new_label14
  {
  k <- 0
  while (k < m1_l2) :new_label15 :new_label22 
  :new_label15
  {
  int64 F9
  int64 F8
  int64 F7
  int64 F6
  int64 F5
  int64 F4
  int64 F3
  int64 F2
  int64 F1
  int64 F0
  int64 E4
  int64 E3
  int64 E2
  int64 E1
  int64 E0
  int64 F
  int64 E
  int64 D
  int64 C
  int64 B
  int64 A
  A <- m1[i][k]
  B <- m2[k][j]
  C <- A * B
  D <- m3[i][j]
  E <- D * 4
  E0 <- E * 3
  E1 <- E0 * 3
  E2 <- E1 * 3
  E3 <- E2 * 3
  E4 <- E3 * 3
  if (E4 >= 1000) :new_label16 :new_label17 
  :new_label16
  {
  E4 <- 10
}


  goto :new_label18
  :new_label17
  {
}


  :new_label18
  F0 <- E4 + 2
  F1 <- F0 << 1
  F2 <- F1 << 1
  F3 <- F2 * 2
  F4 <- F3 + 1
  F5 <- F4 << 1
  F6 <- F5 << 1
  F7 <- F6 * 2
  F8 <- F7 * 2
  if (F8 >= 1500) :new_label19 :new_label20 
  :new_label19
  {
  F8 <- F8 >> 4
}


  goto :new_label21
  :new_label20
  {
}


  :new_label21
  F <- F8 + 1
  m3[i][j] <- F
  k <- k + 1
}


  continue
  :new_label22
  j <- j + 1
}


  continue
  :new_label23
  i <- i + 1
}


  continue
  :new_label24
}





int64 totalSum (int64[][] m)
{
  int64 sum
  int64 index1
  int64 l2
  int64 l1
  l1 <- length m 0
  l2 <- length m 1
  while (index1 < l1) :new_label0 :new_label3 
  :new_label0
  {
  int64 index2
  index2 <- 0
  while (index2 < l2) :new_label1 :new_label2 
  :new_label1
  {
  int64 temp
  temp <- m[index1][index2]
  sum <- sum + temp
  index2 <- index2 + 1
}


  continue
  :new_label2
  index1 <- index1 + 1
}


  continue
  :new_label3
  return sum
}




