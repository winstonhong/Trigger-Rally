
// vmath.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include <math.h>


#define PI 3.1415926535897932384626433832795

#define RADIANS(a) ((a)*PI/180.0)
#define DEGREES(a) ((a)*180.0/PI)

#define rand01 ((float)rand() / (float)RAND_MAX)
#define randm11 (2.0f * (float)rand() / (float)RAND_MAX - 1.0f)

#define MAX(a,b) (((a) >= (b)) ? (a) : (b))
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define CLAMP_UPPER(v,max) v = MIN(v, max)
#define CLAMP_LOWER(v,min) v = MAX(v, min)
#define CLAMP(v,min,max) v = MAX(MIN(v, max), min)
#define INTERP(a,b,f) ((a) + ((b) - (a)) * (f))

#define SQUARED(x) ((x)*(x))

#define PULLTOWARD(val,target,delta) (val = (target) + (val-(target)) / (1.0 + (delta)))

#define DEADZONE(val, zone) (val = \
  (val > (zone)) ? ((val - (zone)) / (1.0 - (zone))) : \
  ((val < (-zone)) ? ((val + (zone)) / (1.0 - (zone))) : 0.0))

#define RANGEADJUST(val, premin, premax, postmin, postmax) (val = \
  ((val - (premin)) * ((postmax) - (postmin)) / ((premax) - (premin)) + (postmin)))

#define makevec2f(vec) (vec2f(vec.x,vec.y))
#define makevec3f(vec) (vec3f(vec.x,vec.y,vec.z))
#define makevec4f(vec) (vec4f(vec.x,vec.y,vec.z,vec.w))

#define makevec4d(vec) (vec4d(vec.x,vec.y,vec.z,vec.w))


template<class T>
class vec2
{
public:
  T x,y;

  vec2 () { }
  vec2 (T _x, T _y) { x=_x; y=_y; }
  vec2 (const vec2<T> &vec) { x=vec.x; y=vec.y; }

  T &operator [] (int i) { return (&x)[i]; }
  operator const T* () const { return &x; }
  operator T* () { return &x; }

  vec2<T> &operator = (const vec2<T> &vec) { x=vec.x; y=vec.y; return *this; }
  vec2<T> &operator += (const vec2<T> &vec) { x+=vec.x; y+=vec.y; return *this; }
  vec2<T> &operator -= (const vec2<T> &vec) { x-=vec.x; y-=vec.y; return *this; }
  vec2<T> &operator *= (T factor) { x*=factor; y*=factor; return *this; }
  vec2<T> &operator /= (const T factor) { x/=factor; y/=factor; return *this; }

  vec2<T> operator + (const vec2<T> &vec) const { return vec2<T>(x+vec.x, y+vec.y); }
  vec2<T> operator - (const vec2<T> &vec) const { return vec2<T>(x-vec.x, y-vec.y); }

  vec2<T> operator * (T factor) const { return vec2<T>(x*factor, y*factor); }
  vec2<T> operator / (T factor) const { return vec2<T>(x/factor, y/factor); }

  T dot (const vec2<T> &vec) const { return (x*vec.x + y*vec.y); }
  T operator * (const vec2<T> &vec) const { return dot(vec); }

  T lengthsq () const { return (x*x + y*y); }
  T length () const { return (T)sqrt(lengthsq()); }
  void normalize () {
    T len = length();
    if (len) {
      x /= len;
      y /= len;
    } else {
      *this = vec2<T>((T)1,(T)0);
    }
  }

  static const vec2<T> zero () { return vec2<T>(0,0); }

  static vec2<T> rand() {
    vec2<T> ret;
    do { ret=vec2<T>((T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1); }
    while (ret.lengthsq() > (T)1);
    return ret;
  }
};

template<class T> inline vec2<T> operator - (const vec2<T> &vec) { return vec2<T>(-vec.x,-vec.y); }


template<class T>
class vec3
{
public:
  T x,y,z;

  vec3 () { }
  vec3 (T _x, T _y, T _z) { x=_x; y=_y; z=_z; }
  vec3 (const vec3<T> &vec) { x=vec.x; y=vec.y; z=vec.z; }
  vec3 (const T *vecptr) { x=vecptr[0]; y=vecptr[1]; z=vecptr[2]; }

  T &operator [] (int i) { return (&x)[i]; }
  operator const T* () const { return &x; }
  operator T* () { return &x; }

  //operator vec2<T> () const { return vec2<T>(x,y); }

  vec3<T> operator + (const vec3<T> &vec) const { return vec3<T>(x+vec.x, y+vec.y, z+vec.z); }
  vec3<T> operator - (const vec3<T> &vec) const { return vec3<T>(x-vec.x, y-vec.y, z-vec.z); }
  vec3<T> operator * (T factor) const { return vec3<T>(x*factor, y*factor, z*factor); }
  vec3<T> operator / (T divisor) const {
    float factor = divisor != ((T)0) ? ((T)1) / divisor : ((T)0);
    return vec3<T>(x*factor, y*factor, z*factor);
  }
  
  vec3<T> &operator += (const vec3<T> &vec) { *this = *this + vec; return *this; }
  vec3<T> &operator -= (const vec3<T> &vec) { *this = *this - vec; return *this; }
  vec3<T> &operator *= (const T factor) { *this = *this * factor; return *this; }
  vec3<T> &operator /= (const T factor) { *this = *this / factor; return *this; }
  

  T dot (const vec3<T> &vec) const { return (x*vec.x + y*vec.y + z*vec.z); }
  T operator * (const vec3<T> &vec) const { return dot(vec); }

  vec3<T> cross (const vec3<T> &vec) const { return vec3<T>(y*vec.z-z*vec.y, z*vec.x-x*vec.z, x*vec.y-y*vec.x); }
  vec3<T> operator ^ (const vec3<T> &vec) const { return cross(vec); }
  
  vec3<T> modulate (const vec3<T> &vec) const { return vec3<T>(x*vec.x, y*vec.y, z*vec.z); }
  
  T lengthsq () const { return (x*x + y*y + z*z); }
  T length () const { return (T)sqrt(lengthsq()); }
  void normalize () {
    T len = length();
    if (len) {
      len = (T)1 / len;
      x *= len;
      y *= len;
      z *= len;
    } else {
      *this = vec3<T>((T)0,(T)0,(T)1);
    }
  }

  void copyTo(T *dest) const { dest[0] = x; dest[1] = y; dest[2] = z; }
  void copyFrom(const T *src) { x = src[0]; y = src[1]; z = src[2]; }

  static vec3<T> zero () { return vec3<T>(0,0,0); }

  static vec3<T> rand() {
    vec3<T> ret;
    do { ret=vec3<T>((T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1); }
    while (ret.lengthsq() > (T)1);
    return ret;
  }
};

template<class T> inline vec3<T> operator - (const vec3<T> &vec) { return vec3<T>(-vec.x,-vec.y,-vec.z); }

template<class T> inline vec3<T> operator * (const T &fac, const vec3<T> &vec) { return vec * fac; }

template<class T>
class vec4
{
public:
  T x,y,z,w;

  vec4 () { }
  vec4 (T _x, T _y, T _z, T _w) { x=_x; y=_y; z=_z; w=_w; }
  vec4 (const vec3<T> &vec) { x=vec.x; y=vec.y; z=vec.z; }
  vec4 (const vec4<T> &vec) { x=vec.x; y=vec.y; z=vec.z; w=vec.w; }
  vec4 (const vec3<T> &vec, T _w) { x=vec.x; y=vec.y; z=vec.z; w=_w; }

  T &operator [] (int i) { return (&x)[i]; }
  operator const T* () const { return &x; }
  operator T* () { return &x; }

  //operator vec3<T> () const { return vec3<T>(x,y,z); }

  vec4<T> operator + (const vec4<T> &vec) const { return vec4<T>(x+vec.x, y+vec.y, z+vec.z, w+vec.w); }
  vec4<T> operator - (const vec4<T> &vec) const { return vec4<T>(x-vec.x, y-vec.y, z-vec.z, w-vec.w); }
  vec4<T> operator * (T factor) const { return vec4<T>(x*factor, y*factor, z*factor, w*factor); }
  
  vec4<T> &operator += (const vec4<T> &vec) { *this = *this + vec; return *this; }
  vec4<T> &operator -= (const vec4<T> &vec) { *this = *this - vec; return *this; }
  vec4<T> &operator *= (const T factor) { *this = *this * factor; return *this; }
  
  T dot (const vec4<T> &vec) const { return (x*vec.x + y*vec.y + z*vec.z + w*vec.w); }
  T operator * (const vec4<T> &vec) const { return dot(vec); }
  
  T lengthsq () const { return (x*x + y*y + z*z + w*w); }
  T length () const { return sqrt(lengthsq()); }
  void normalize3a () {
    T len = sqrt(x*x + y*y + z*z);
    if (len) {
      len = (T)1 / len;
      x *= len;
      y *= len;
      z *= len;
      w *= len;
    } else {
      *this = vec4<T>((T)0,(T)0,(T)1,(T)0);
    }
  }
  void normalize3b () {
    if (w) {
      T len = (T)1 / w;
      x *= len;
      y *= len;
      z *= len;
      w = (T)1;
    } else {
      *this = vec4<T>((T)0,(T)0,(T)0,(T)1);
    }
  }
  void normalize3c () {
    T len = sqrt(x*x + y*y + z*z);
    if (len) {
      len = (T)1 / len;
      x *= len;
      y *= len;
      z *= len;
    } else {
      x = (T)0;
      y = (T)0;
      z = (T)1;
    }
  }
  void normalize4 () {
    T len = length();
    if (len) {
      len = (T)1 / len;
      x *= len;
      y *= len;
      z *= len;
      w *= len;
    } else {
      *this = vec4<T>((T)0,(T)0,(T)0,(T)1);
    }
  }

  void copyTo(T *dest) const { dest[0] = x; dest[1] = y; dest[2] = z; dest[3] = w; }
  void copyFrom(const T *src) { x = src[0]; y = src[1]; z = src[2]; w = src[3]; }
};

template<class T>
class ray
{
public:
  vec3<T> start;
  vec3<T> direction;
  
  ray () { }
  ray (const vec3<T> &_start, const vec3<T> &_direction) : start(_start), direction(_direction) { }

  void fromTwoPoints(const vec3<T> &_start, const vec3<T> &_pt) {
    start = _start;
    direction = _pt - _start;
  }
  
  vec3<T> getPt(T lambda) const {
    return start + direction * lambda;
  }
  
  bool getSphereIntersection(const vec3<T> &center, T radius, T &outLambda) const
  {
    vec3<T> temp = start - center;

    T a = direction.lengthsq(),
    b = 2.0 * (direction * temp),
    c = temp.lengthsq() - radius*radius,
    determinant = b * b - (T)4 * a * c;

    if (determinant < (T)0) return false;

    T rootdet = sqrt(determinant),
    t = (-b - rootdet) / ((T)2 * a);

    outLambda = t;
    return true;
  }
};

// this plane class is of form (x,y,z) . normal + offset = 0
// ie, increasing offset moves plane backwards along its normal
template<class T>
class plane
{
public:
  vec3<T> normal;
  T offset;

  plane () { }
  plane (const vec3<T> &_normal, T _offset) : normal(_normal), offset(_offset) { }
  plane (T _a, T _b, T _c, T _d) : normal(_a, _b, _c), offset(_d) { }

  void normalize() {
    T len = normal.length();
    if (len) {
      len = (T)1 / len;
      normal *= len;
      offset *= len;
    }
  }
};

template<class T>
class mat44
{
public:
  vec4<T> row[4];

  mat44 () { }

  T &operator [] (int i) { return row[0][i]; }
  operator T* () { return &row[0].x; }
  operator const T* () const { return &row[0].x; }

  mat44<T> &assemble(const vec4<T> &r0, const vec4<T> &r1, const vec4<T> &r2, const vec4<T> &r3) {
    row[0] = r0; row[1] = r1; row[2] = r2; row[3] = r3;
    return *this;
  }
  mat44<T> &assemble(const vec3<T> &r0, const vec3<T> &r1, const vec3<T> &r2) {
    row[0] = vec4<T>(r0.x, r0.y, r0.z, (T)0);
    row[1] = vec4<T>(r1.x, r1.y, r1.z, (T)0);
    row[2] = vec4<T>(r2.x, r2.y, r2.z, (T)0);
    row[3] = vec4<T>((T)0, (T)0, (T)0, (T)1);
    return *this;
  }

  mat44<T> concatenate(const mat44<T> &mat) const {
    mat44<T> ret;
    ret.row[0][0] = row[0][0]*mat.row[0][0] + row[0][1]*mat.row[1][0] + row[0][2]*mat.row[2][0] + row[0][3]*mat.row[3][0];
    ret.row[0][1] = row[0][0]*mat.row[0][1] + row[0][1]*mat.row[1][1] + row[0][2]*mat.row[2][1] + row[0][3]*mat.row[3][1];
    ret.row[0][2] = row[0][0]*mat.row[0][2] + row[0][1]*mat.row[1][2] + row[0][2]*mat.row[2][2] + row[0][3]*mat.row[3][2];
    ret.row[0][3] = row[0][0]*mat.row[0][3] + row[0][1]*mat.row[1][3] + row[0][2]*mat.row[2][3] + row[0][3]*mat.row[3][3];
    ret.row[1][0] = row[1][0]*mat.row[0][0] + row[1][1]*mat.row[1][0] + row[1][2]*mat.row[2][0] + row[1][3]*mat.row[3][0];
    ret.row[1][1] = row[1][0]*mat.row[0][1] + row[1][1]*mat.row[1][1] + row[1][2]*mat.row[2][1] + row[1][3]*mat.row[3][1];
    ret.row[1][2] = row[1][0]*mat.row[0][2] + row[1][1]*mat.row[1][2] + row[1][2]*mat.row[2][2] + row[1][3]*mat.row[3][2];
    ret.row[1][3] = row[1][0]*mat.row[0][3] + row[1][1]*mat.row[1][3] + row[1][2]*mat.row[2][3] + row[1][3]*mat.row[3][3];
    ret.row[2][0] = row[2][0]*mat.row[0][0] + row[2][1]*mat.row[1][0] + row[2][2]*mat.row[2][0] + row[2][3]*mat.row[3][0];
    ret.row[2][1] = row[2][0]*mat.row[0][1] + row[2][1]*mat.row[1][1] + row[2][2]*mat.row[2][1] + row[2][3]*mat.row[3][1];
    ret.row[2][2] = row[2][0]*mat.row[0][2] + row[2][1]*mat.row[1][2] + row[2][2]*mat.row[2][2] + row[2][3]*mat.row[3][2];
    ret.row[2][3] = row[2][0]*mat.row[0][3] + row[2][1]*mat.row[1][3] + row[2][2]*mat.row[2][3] + row[2][3]*mat.row[3][3];
    ret.row[3][0] = row[3][0]*mat.row[0][0] + row[3][1]*mat.row[1][0] + row[3][2]*mat.row[2][0] + row[3][3]*mat.row[3][0];
    ret.row[3][1] = row[3][0]*mat.row[0][1] + row[3][1]*mat.row[1][1] + row[3][2]*mat.row[2][1] + row[3][3]*mat.row[3][1];
    ret.row[3][2] = row[3][0]*mat.row[0][2] + row[3][1]*mat.row[1][2] + row[3][2]*mat.row[2][2] + row[3][3]*mat.row[3][2];
    ret.row[3][3] = row[3][0]*mat.row[0][3] + row[3][1]*mat.row[1][3] + row[3][2]*mat.row[2][3] + row[3][3]*mat.row[3][3];
    return ret;
  }
  
  mat44<T> concatenateReverse(const mat44<T> &mat) const {
    return mat.concatenate(*this);
  }
  
  mat44<T> transpose() const {
    mat44<T> ret;
    ret.row[0] = vec4<T>(row[0].x, row[1].x, row[2].x, row[3].x);
    ret.row[1] = vec4<T>(row[0].y, row[1].y, row[2].y, row[3].y);
    ret.row[2] = vec4<T>(row[0].z, row[1].z, row[2].z, row[3].z);
    ret.row[3] = vec4<T>(row[0].w, row[1].w, row[2].w, row[3].w);
    return ret;
  }

  mat44<T> &translate(const vec3<T> &vec) {
    row[0].w += vec.x;
    row[1].w += vec.y;
    row[2].w += vec.z;
    return *this;
  }
  
  vec3<T> transform1(const vec3<T> &vec) const {
    return vec3<T> (
      row[0].x * vec.x + row[0].y * vec.y + row[0].z * vec.z + row[0].w,
      row[1].x * vec.x + row[1].y * vec.y + row[1].z * vec.z + row[1].w,
      row[2].x * vec.x + row[2].y * vec.y + row[2].z * vec.z + row[2].w
      );
  }

  vec4<T> transform1(const vec4<T> &vec) const {
    return vec4<T> (
      row[0].x * vec.x + row[0].y * vec.y + row[0].z * vec.z + row[0].w * vec.w,
      row[1].x * vec.x + row[1].y * vec.y + row[1].z * vec.z + row[1].w * vec.w,
      row[2].x * vec.x + row[2].y * vec.y + row[2].z * vec.z + row[2].w * vec.w,
      row[3].x * vec.x + row[3].y * vec.y + row[3].z * vec.z + row[3].w * vec.w
      );
  }

  vec3<T> transform2(const vec3<T> &vec) const {
    return vec3<T> (
      row[0].x * vec.x + row[1].x * vec.y + row[2].x * vec.z + row[3].x,
      row[0].y * vec.x + row[1].y * vec.y + row[2].y * vec.z + row[3].y,
      row[0].z * vec.x + row[1].z * vec.y + row[2].z * vec.z + row[3].z
      );
  }

  vec4<T> transform2(const vec4<T> &vec) const {
    return vec4<T> (
      row[0].x * vec.x + row[1].x * vec.y + row[2].x * vec.z + row[3].x * vec.w,
      row[0].y * vec.x + row[1].y * vec.y + row[2].y * vec.z + row[3].y * vec.w,
      row[0].z * vec.x + row[1].z * vec.y + row[2].z * vec.z + row[3].z * vec.w,
      row[0].w * vec.x + row[1].w * vec.y + row[2].w * vec.z + row[3].w * vec.w
      );
  }

  vec3<T> transformNormal(const vec3<T> &vec) const {
    return vec3<T> (
      row[0].x * vec.x + row[0].y * vec.y + row[0].z * vec.z,
      row[1].x * vec.x + row[1].y * vec.y + row[1].z * vec.z,
      row[2].x * vec.x + row[2].y * vec.y + row[2].z * vec.z
      );
  }

  static mat44<T> identity() {
    mat44<T> ret;
    ret.row[0] = vec4<T>(1,0,0,0);
    ret.row[1] = vec4<T>(0,1,0,0);
    ret.row[2] = vec4<T>(0,0,1,0);
    ret.row[3] = vec4<T>(0,0,0,1);
    return ret;
  }
};


template <class T>
class quat {
public:
  T x,y,z,w;

public:
  quat() { }
  quat(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }

  quat<T> operator + (const quat<T> &q) const {
    return quat<T>(x+q.x, y+q.y, z+q.z, w+q.w);
  }
  quat<T> operator - (const quat<T> &q) const {
    return quat<T>(x-q.x, y-q.y, z-q.z, w-q.w);
  }
  quat<T> operator * (T val) const {
    return quat<T>(x*val, y*val, z*val, w*val);
  }
  quat<T> operator / (T val) const {
    T mult = (T)1 / val;
    return quat<T>(x*mult, y*mult, z*mult, w*mult);
  }

  quat<T> operator * (const quat<T> &q) const {
    return quat<T>(
      w*q.x + x*q.w + y*q.z - z*q.y,
      w*q.y - x*q.z + y*q.w + z*q.x,
      w*q.z + x*q.y - y*q.x + z*q.w,
      w*q.w - x*q.x - y*q.y - z*q.z);
  }

  quat<T> &operator += (const quat<T> &q) {
    *this = *this + q;
    return *this;
  }
  // operator *= has to assume an order
  // since quaternion mult is not commutative, it's
  // better to force the user to think about it
  
  float dot(const quat<T> &q) {
    return (x * q.x + y * q.y + z * q.z + w * q.w);
  }

  void fromAxisAngle(const vec3<T> &axis, T angle) {
    T len = axis.length();
    if (len > (T)0) {
      T sinA = sin(angle/(T)2) / len;
      x = sinA * axis.x;
      y = sinA * axis.y;
      z = sinA * axis.z;
      w = cos(angle/(T)2);
    } else {
      *this = identity();
    }
  }
  void fromXAngle(T angle) {
    x = sin(angle/(T)2);
    y = z = (T)0;
    w = cos(angle/(T)2);
  }
  void fromYAngle(T angle) {
    y = sin(angle/(T)2);
    x = z = (T)0;
    w = cos(angle/(T)2);
  }
  void fromZAngle(T angle) {
    z = sin(angle/(T)2);
    x = y = (T)0;
    w = cos(angle/(T)2);
  }
  void fromThreeAxisAngle(const vec3<T> &vec) {
    quat<T> temp;
    fromXAngle(vec.x);
    temp.fromYAngle(vec.y);
    *this = *this * temp;
    temp.fromZAngle(vec.z);
    *this = *this * temp;
  }

  void normalize() {
    T len = x*x + y*y + z*z + w*w;
    if (len > (T)0) {
      len = (T)1 / (T)sqrt(len);
      x *= len;
      y *= len;
      z *= len;
      w *= len;
    } else {
      *this = identity();
    }
  }

  mat44<T> getMatrix() const {
    T   norm = x*x + y*y + z*z + w*w,
      s = (norm > (T)0) ? (T)2/norm : (T)0,

      xx = x * x * s,
      yy = y * y * s,
      zz = z * z * s,
      xy = x * y * s,
      xz = x * z * s,
      yz = y * z * s,
      wx = w * x * s,
      wy = w * y * s,
      wz = w * z * s;

    mat44<T> m;
    m.assemble(
      vec3<T>((T)1 - (yy + zz), xy + wz, xz - wy),
      vec3<T>(xy - wz, (T)1 - (xx + zz), yz + wx),
      vec3<T>(xz + wy, yz - wx, (T)1 - (xx + yy)));

    return m;
/*
    m.el[0] = (T)1 - (yy + zz);
    m.el[4] = xy + wz;
    m.el[8] = xz - wy;

    m.el[1] = xy - wz;
    m.el[5] = (T)1 - (xx + zz);
    m.el[9] = yz + wx;

    m.el[2] = xz + wy;
    m.el[6] = yz - wx;
    m.el[10] = (T)1 - (xx + yy);

    m.el[12] = m.el[13] = m.el[14] = m.el[3] = m.el[7] = m.el[11] = (T)0;
    m.el[15] = (T)1;
*/
  }

  static quat<T> identity () { return quat<T>(0,0,0,1); }

  static quat<T> rand() {
    quat<T> ret;
    float lengthsq;
    do { ret=quat<T>((T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1,
            (T)2 * (T)::rand() / (T)RAND_MAX - (T)1);
      lengthsq = ret.x*ret.x + ret.y*ret.y + ret.z*ret.z + ret.w*ret.w;
    } while (lengthsq > (T)1 || lengthsq <= (T)0);
    float mult = (T)1 / (T)sqrt(lengthsq);
    ret.x *= mult; ret.y *= mult; ret.z *= mult; ret.w *= mult;
    return ret;
  }
};

template<class T>
class frustum {
public:

  enum {
    SideRight,
    SideLeft,
    SideTop,
    SideBottom,
    SideNear,
    SideFar,
    SideCount
  };

  plane<T> side[SideCount];

public:
  frustum() { }
  frustum(const mat44<T> &mat) { construct(mat); }

  frustum<T> &construct(const mat44<T> &mat);

  bool isAABBOutside(const vec3<T> &mins, const vec3<T> &maxs) const;
};


typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
typedef vec2<int> vec2i;
typedef vec2<unsigned char> vec2ub;

typedef vec3<float> vec3f;
typedef vec3<double> vec3d;
typedef vec3<int> vec3i;
typedef vec3<unsigned char> vec3ub;

typedef vec4<float> vec4f;
typedef vec4<double> vec4d;
typedef vec4<int> vec4i;
typedef vec4<unsigned char> vec4ub;

typedef quat<float> quatf;
typedef quat<double> quatd;

typedef ray<float> rayf;
typedef ray<double> rayd;

typedef plane<float> planef;
typedef plane<double> planed;

typedef mat44<float> mat44f;
typedef mat44<double> mat44d;

typedef frustum<float> frustumf;
typedef frustum<double> frustumd;

template<class T> inline T intersect_plane_ray(const plane<T> &pln, const vec3<T> &linept, const vec3<T> &linedir)
{
  T denom = pln.normal * linedir;
  if (denom) {
    return - (pln.normal * linept + pln.offset) / denom;
  } else {
    return (T)0;
  }
}



