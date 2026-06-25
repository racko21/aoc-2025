package main
import ("fmt")
func rotate2(cells [][2]int) [][2]int { out:=make([][2]int,len(cells)); for i,p:=range cells{out[i]=[2]int{p[1],-p[0]}}; return out }
func flip2(cells [][2]int) [][2]int { out:=make([][2]int,len(cells)); for i,p:=range cells{out[i]=[2]int{p[0],-p[1]}}; return out }
func norm2(cells [][2]int)[][2]int{
 mr,mc:=1<<30,1<<30
 for _,p:=range cells{if p[0]<mr{mr=p[0]};if p[1]<mc{mc=p[1]}}
 out:=make([][2]int,len(cells))
 for i,p:=range cells{out[i]=[2]int{p[0]-mr,p[1]-mc}}
 return out
}
func main(){
 base:=[][2]int{{0,0},{0,1},{0,2},{1,0},{2,0},{2,1},{2,2}}
 var oris [][][2]int
 cur:=base
 for f:=0;f<2;f++{
  c:=cur
  for r:=0;r<4;r++{ oris=append(oris,norm2(c)); c=rotate2(c) }
  cur=flip2(cur)
 }
 fmt.Println("oris",len(oris))
 W,H:=4,4
 grid:=make([]bool,16)
 var solve func(int,int)bool
 solve=func(placed,sp int)bool{
  if placed==2{return true}
  pos:=-1
  for p:=sp;p<16;p++{if !grid[p]{pos=p;break}}
  if pos<0{return false}
  pr,pc:=pos/W,pos%W
  for _,ori:=range oris{
   for _,anchor:=range ori{
    or0,oc0:=anchor[0],anchor[1]
    ok:=true
    for _,cell:=range ori{
     rr:=pr+(cell[0]-or0); cc:=pc+(cell[1]-oc0)
     if rr<0||rr>=H||cc<0||cc>=W||grid[rr*W+cc]{ok=false;break}
    }
    if !ok{continue}
    for _,cell:=range ori{rr:=pr+(cell[0]-or0);cc:=pc+(cell[1]-oc0);grid[rr*W+cc]=true}
    if solve(placed+1,pos){return true}
    for _,cell:=range ori{rr:=pr+(cell[0]-or0);cc:=pc+(cell[1]-oc0);grid[rr*W+cc]=false}
   }
  }
  return false
 }
 fmt.Println(solve(0,0))
}
