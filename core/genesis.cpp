#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// GENESIS V0.1: controlled hidden-law discovery benchmark.
// The candidate feature library is explicit; this is not open-ended discovery.
static double hidden_law(double x) {
    double signal = 0.7*x*x + 1.4*std::sin(1.7*x) + 0.3*x + 2.0;
    double noise = 0.03*std::sin(11.0*x) + 0.02*std::cos(7.0*x);
    return signal + noise;
}
static double median(std::vector<double> v) {
  if(v.empty()) return 0;
  std::sort(v.begin(),v.end()); size_t n=v.size();
  return n%2?v[n/2]:(v[n/2-1]+v[n/2])/2.0;
}
static double rmse(const std::vector<double>& a,const std::vector<double>& b) {
  if(a.empty()||a.size()!=b.size()) return 1e9;
  double s=0; for(size_t i=0;i<a.size();++i){double d=a[i]-b[i];s+=d*d;}
  return std::sqrt(s/a.size());
}
// Fit linear coefficients for a chosen feature set using small ridge-stabilized
// normal-equation Gaussian elimination. Features: 1, x, x^2, sin(1.7x), cos(1.7x).
static std::vector<double> fit(const std::vector<double>& xs,const std::vector<double>& ys,const std::vector<int>& fs) {
  int n=(int)fs.size(); std::vector<std::vector<double>> a(n,std::vector<double>(n+1,0));
  auto phi=[](int f,double x){switch(f){case 0:return 1.0;case 1:return x;case 2:return x*x;case 3:return std::sin(1.7*x);default:return std::cos(1.7*x);}};
  for(int i=0;i<n;i++){for(int j=0;j<n;j++)for(size_t k=0;k<xs.size();k++)a[i][j]+=phi(fs[i],xs[k])*phi(fs[j],xs[k]);
    for(size_t k=0;k<xs.size();k++)a[i][n]+=phi(fs[i],xs[k])*ys[k]; a[i][i]+=1e-9;}
  for(int i=0;i<n;i++){int p=i;for(int r=i+1;r<n;r++)if(std::abs(a[r][i])>std::abs(a[p][i]))p=r;std::swap(a[i],a[p]);
    double d=a[i][i];if(std::abs(d)<1e-12)return std::vector<double>(n,0);for(int c=i;c<=n;c++)a[i][c]/=d;
    for(int r=0;r<n;r++)if(r!=i){double q=a[r][i];for(int c=i;c<=n;c++)a[r][c]-=q*a[i][c];}}
  std::vector<double> w(n);for(int i=0;i<n;i++)w[i]=a[i][n];return w;
}
static double predict(double x,const std::vector<int>& fs,const std::vector<double>& w){double y=0;for(size_t i=0;i<fs.size();i++){double z=0;switch(fs[i]){case 0:z=1;break;case 1:z=x;break;case 2:z=x*x;break;case 3:z=std::sin(1.7*x);break;case 4:z=std::cos(1.7*x);break;}y+=w[i]*z;}return y;}

extern "C" {
const char* genesis_version(){return "GENESIS V0.1 · C++20";}
const char* genesis_run(){
  static std::string out;
  std::vector<double> tx,ty,hx,hy;
  // Deterministic train/holdout samples; fixed seed-like sequence for reproducibility.
  for(int i=0;i<72;i++){double x=-3.0+6.0*(i+0.37)/72.0;tx.push_back(x);ty.push_back(hidden_law(x));}
  for(int i=0;i<120;i++){double x=-3.0+6.0*(i+0.5)/120.0;hx.push_back(x);hy.push_back(hidden_law(x));}
  std::vector<int> baseline{0,1}; auto wb=fit(tx,ty,baseline);std::vector<double> bp;
  for(double x:hx)bp.push_back(predict(x,baseline,wb));double base=rmse(hy,bp);
  std::vector<std::vector<int>> candidates={{0,1},{0,1,2},{0,1,3},{0,1,2,3},{0,1,2,3,4}};
  double best=1e99;std::vector<int> bestf;std::vector<double> bestw;
  for(auto fs:candidates){auto w=fit(tx,ty,fs);std::vector<double> p;for(double x:hx)p.push_back(predict(x,fs,w));double e=rmse(hy,p);if(e<best){best=e;bestf=fs;bestw=w;}}
  std::ostringstream s;s<<std::fixed<<std::setprecision(6);
  s<<"{\"version\":\"GENESIS V0.1 · C++20\",\"training_readings\":"<<tx.size()<<",\"holdout_readings\":"<<hx.size()<<",\"baseline_rmse\":"<<base<<",\"genesis_rmse\":"<<best<<",\"improvement_percent\":"<<(base>0?(1-best/base)*100:0)<<",\"selected_features\":[";
  for(size_t i=0;i<bestf.size();i++){if(i)s<<",";const char* names[]={"constant","x","x_squared","sin_1.7x","cos_1.7x"};s<<"\""<<names[bestf[i]]<<"\"";}
  s<<"],\"holdout_pass\":"<<(best<0.05?"true":"false")<<",\"scientific_note\":\"Controlled synthetic benchmark. Candidate features were predefined; this is not evidence of new physics or unrestricted discovery.\"}";
  out=s.str();return out.c_str();
}
}
