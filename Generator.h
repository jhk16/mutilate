// -*- c++ -*-

// 1. implement "fixed" generator
// 2. implement discrete generator
// 3. implement combine generator? 

#ifndef GENERATOR_H
#define GENERATOR_H

#define MAX(a,b) ((a) > (b) ? (a) : (b))

#include "config.h"

#include <string>
#include <vector>
#include <utility>

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"

#ifdef TRACE
#include <bits/stdc++.h>
#endif

// Generator syntax:
//
// \d+ == fixed
// n[ormal]:mean,sd
// e[xponential]:lambda
// p[areto]:scale,shape
// g[ev]:loc,scale,shape
// fb_value, fb_key, fb_rate

class Generator {
public:
  Generator() {}
  //  Generator(const Generator &g) = delete;
  //  virtual Generator& operator=(const Generator &g) = delete;
  virtual ~Generator() {}

  virtual double generate(double U = -1.0) = 0;
  virtual void set_lambda(double lambda) {DIE("set_lambda() not implemented");}
protected:
  std::string type;
};

class Fixed : public Generator {
public:
  Fixed(double _value = 1.0) : value(_value) { D("Fixed(%f)", value); }
  virtual double generate(double U = -1.0) { return value; }
  virtual void set_lambda(double lambda) {
    if (lambda > 0.0) value = 1.0 / lambda;
    else value = 0.0;
  }

private:
  double value;
};

class Uniform : public Generator {
public:
  Uniform(double _scale) : scale(_scale) { D("Uniform(%f)", scale); }

  virtual double generate(double U = -1.0) {
    if (U < 0.0) U = drand48();
    return scale * U;
  }

  virtual void set_lambda(double lambda) {
    if (lambda > 0.0) scale = 2.0 / lambda;
    else scale = 0.0;
  }

private:
  double scale;
};

class Zipf : public Generator {
public:
  Zipf(int _n, double _theta, long permutation_seed) : n(_n), theta(_theta) {
    this->alpha = 1.0 / (1.0 - this->theta);
    this->zetan = this->zeta(this->n, this->theta);
    this->eta = (1.0 - pow(2.0 / n, 1.0 - theta)) / (1.0 - this->zeta(2.0, this->theta) / this->zetan);
    this->permutation = new int[this->n];
    D("Zipf(n=%d, theta=%lf, seed=%ld)", n, theta, permutation_seed);
#ifdef TRACE
    this->popular = new int[this->n]{};
    this->seed = permutation_seed;
#endif
    for (int i=0; i<n; i++)
      this->permutation[i] = i;
    if (permutation_seed) {
      drand48_data drand_buf;
      srand48_r(permutation_seed, &drand_buf);
      for (int i=n-1; i>0; i--) {
        long j;
        lrand48_r(&drand_buf, &j);
        j %= i + 1;
        int tmp = this->permutation[i];
        this->permutation[i] = this->permutation[j];
        this->permutation[j] = tmp;
      }
    }
  }

  ~Zipf() {
#ifdef TRACE
    FILE *log;
    char log_name[256];
    snprintf(log_name, 256, "zipf%.2lf-%d-%ld.log", this->theta, this->n, this->seed);
    FILE *zlog;
    char zlog_name[256];
    snprintf(zlog_name, 256, "zipf%.2lf-%d-%ld.zipf.log", this->theta, this->n, this->seed);
    FILE *slog;
    char slog_name[256];
    snprintf(slog_name, 256, "zipf%.2lf-%d-%ld.sort.log", this->theta, this->n, this->seed);

    zlog = fopen(zlog_name, "w+");
    log = fopen(log_name, "w+");
    for (int i=0; i<this->n;i++) {
      fprintf(zlog, "%d,%d\n", permutation[i], this->popular[permutation[i]]);
      fprintf(log, "%d,%d\n", i, this->popular[i]);
    }
    fclose(log);
    fclose(zlog);

    slog = fopen(slog_name, "w+");
    std::sort(this->popular, this->popular + this->n);
    for (int i=0; i < this->n; i++)
      fprintf(slog, "%d,%d\n", i, this->popular[i]);
    fclose(slog);

    delete[] this->popular;
#endif
    delete[] this->permutation;
  }

  virtual double generate(double U = -1.0) {
    int idx;

    if (U < 0.0) U = drand48();
    idx = (int)(1.0 * this->n * pow(this->eta*U - this->eta + 1.0, this->alpha));

    assert(idx >= 0 && idx < this->n);
#ifdef TRACE
    this->popular[permutation[idx]]++;
#endif
    // printf("%d\n", permutation[idx]);
    return permutation[idx];
  }

private:
  int n;
  double theta;
  double alpha;
  double zetan;
  double eta;
  int *permutation;
#ifdef TRACE
  int *popular;
  long seed;
#endif

  double zeta(int n, double theta) {
    double ret = 0;
    for (int i=1; i<=n; i++)
      ret += pow(i, -theta);
    return ret;
  }
};

/* Referred YCSB HotspotIntegerGenerator.java */
class Hotspot : public Generator {
public:
  Hotspot(long _scale, double _hot_key_ratio = 0.1,
		  double _hot_access_ratio = 0.9, long _nset = 10) :
	  scale(_scale), hot_key_ratio(_hot_key_ratio),
	  hot_access_ratio(_hot_access_ratio), nset(_nset) {

    V("Hotspot(scale=%d, hotset=%f, hotopn=%f, nset=%d)", scale,
		    hot_key_ratio, hot_access_ratio, nset);

    if (nset == 0) scale_chunk = 0;
    else scale_chunk = scale / nset;
  }

  virtual double generate(double U = -1.0) {
    long hotchunk;
    long hotstart;
    long V;
    if (U < 0.0) U = drand48();

    if (U < hot_access_ratio) {
      V = lrand48();
      hotchunk = V % nset;
      hotstart = scale * hotchunk / nset;
      return hotstart + (lrand48() % (int)(scale_chunk * hot_key_ratio));
    } else {
      return scale * drand48();
    }
  }

  virtual void set_lambda(double lambda) {
    if (lambda > 0.0) scale = 2.0 / lambda;
    else scale = 0.0;
  }

private:
  long scale;
  long scale_chunk;
  double hot_key_ratio;
  double hot_access_ratio;
  long nset;
};


class Normal : public Generator {
public:
  Normal(double _mean = 1.0, double _sd = 1.0) : mean(_mean), sd(_sd) {
    D("Normal(mean=%f, sd=%f)", mean, sd);
  }

  virtual double generate(double U = -1.0) {
    if (U < 0.0) U = drand48();
    double V = U; // drand48();
    double N = sqrt(-2 * log(U)) * cos(2 * M_PI * V);
    return mean + sd * N;
  }

  virtual void set_lambda(double lambda) {
    if (lambda > 0.0) mean = 1.0 / lambda;
    else mean = 0.0;
  }

private:
  double mean, sd;
};

class Exponential : public Generator {
public:
  Exponential(double _lambda = 1.0) : lambda(_lambda) {
    D("Exponential(lambda=%f)", lambda);
  }

  virtual double generate(double U = -1.0) {
    if (lambda <= 0.0) return 0.0;
    if (U < 0.0) U = drand48();
    return -log(U) / lambda;
  }

  virtual void set_lambda(double lambda) { this->lambda = lambda; }

private:
  double lambda;
};

class GPareto : public Generator {
public:
  GPareto(double _loc = 0.0, double _scale = 1.0, double _shape = 1.0) :
    loc(_loc), scale(_scale), shape(_shape) {
    assert(shape != 0.0);
    D("GPareto(loc=%f, scale=%f, shape=%f)", loc, scale, shape);
  }

  virtual double generate(double U = -1.0) {
    if (U < 0.0) U = drand48();
    return loc + scale * (pow(U, -shape) - 1) / shape;
  }

  virtual void set_lambda(double lambda) {
    if (lambda <= 0.0) scale = 0.0;
    else scale = (1 - shape) / lambda - (1 - shape) * loc;
  }

private:
  double loc /* mu */;
  double scale /* sigma */, shape /* k */;
};

class GEV : public Generator {
public:
  GEV(double _loc = 0.0, double _scale = 1.0, double _shape = 1.0) :
    e(1.0), loc(_loc), scale(_scale), shape(_shape) {
    assert(shape != 0.0);
    D("GEV(loc=%f, scale=%f, shape=%f)", loc, scale, shape);
  }

  virtual double generate(double U = -1.0) {
    return loc + scale * (pow(e.generate(U), -shape) - 1) / shape;
  }

private:
  Exponential e;
  double loc /* mu */, scale /* sigma */, shape /* k */;
};

class Discrete : public Generator {
public:
  ~Discrete() { delete def; }
  Discrete(Generator* _def = NULL) : def(_def) {
    if (def == NULL) def = new Fixed(0.0);
  }

  virtual double generate(double U = -1.0) {
    double Uc = U;
    if (pv.size() > 0 && U < 0.0) U = drand48();

    double sum = 0;
 
    for (auto p: pv) {
      sum += p.first;
      if (U < sum) return p.second;
    }

    return def->generate(Uc);
  }

  void add(double p, double v) {
    pv.push_back(std::pair<double,double>(p, v));
  }

private:
  Generator *def;
  std::vector< std::pair<double,double> > pv;
};

#define KEY_MAX_LENGTH 250

class KeyGenerator {
public:
  KeyGenerator(Generator* _g, double _max = 10000) : g(_g), max(_max) {}
  std::string generate(uint64_t ind) {
    uint64_t h = fnv_64(ind);
    double U = (double) h / ULLONG_MAX;
    double G = g->generate(U);
    int keylen = MAX(round(G), floor(log10(max)) + 1);
    char key[KEY_MAX_LENGTH];
    snprintf(key, KEY_MAX_LENGTH, "%0*" PRIu64, keylen, ind);

    //    D("%d = %s", ind, key);
    return std::string(key);
  }
private:
  Generator* g;
  double max;
};


Generator* createGenerator(std::string str);
Generator *createPopularityGenerator(std::string str, long records, long permutation_seed);
Generator* createFacebookKey();
Generator* createFacebookValue();
Generator* createFacebookIA();

#endif // GENERATOR_H
