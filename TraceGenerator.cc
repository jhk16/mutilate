#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <inttypes.h>
#include <limits.h>
#include <math.h>

#include "Generator.h"
#include "util.h"

struct trace_data {
  unsigned int key_num;
  size_t key_sz;
  int val_sz;
};

char random_char[10 * 1024 * 1024];  // Buffer used to generate random values.

void init_random_stuff() {
  static char lorem[] =
    R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas turpis dui, suscipit non vehicula non, malesuada id sem. Phasellus suscipit nisl ut dui consectetur ultrices tincidunt eros aliquet. Donec feugiat lectus sed nibh ultrices ultrices. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Mauris suscipit eros sed justo lobortis at ultrices lacus molestie. Duis in diam mi. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut cursus viverra sagittis. Vivamus non facilisis tortor. Integer lectus arcu, sagittis et eleifend rutrum, condimentum eget sem. Vestibulum tempus tellus non risus semper semper. Morbi molestie rhoncus mi, in egestas dui facilisis et.)";

  size_t cursor = 0;

  while (cursor < sizeof(random_char)) {
    size_t max = sizeof(lorem);
    if (sizeof(random_char) - cursor < max)
      max = sizeof(random_char) - cursor;

    memcpy(&random_char[cursor], lorem, max);
    cursor += max;
  }
}

int main(int argc, char **argv) {
  //  double now = get_time();
  //  uint64_t x = fnv_64_buf(&now, sizeof(now));

  srand48(0xdeadbeef);
  init_random_stuff();

  /*
  Generator *n = createGenerator("n:1,1"); // new Normal(1, 1);
  Generator *e = createGenerator("e:1"); // new Exponential(1);
  Generator *p = createGenerator("p:214.476,0.348238"); // new GPareto(214.476, 0.348238);
  Generator *g = createGenerator("g:30.7984,8.20449,0.078688"); // new GEV(30.7984, 8.20449, 0.078688);

  printf("%f\n", n->generate());
  printf("%f\n", e->generate());
  printf("%f\n", p->generate());
  printf("%f\n", g->generate());

  srand48(0);

  printf("\n\n");

  Discrete *d = new Discrete(createGenerator("p:214.476,0.348238"));
  //  d->add(.5, -1.0);
  //Generator *d = createGenerator("p:214.476,0.348238");

  for (int i = 0; i < 20; i++) {
        printf("d %d\n", (int) d->generate());
  }

  printf("\n\n");
  srand48(0);

  //Discrete *d2 = new Discrete(createGenerator("p:214.476,0.348238"));
  //  d->add(.5, -1.0);
  Generator *d2 = createGenerator("p:214.476,0.348238");

  for (int i = 0; i < 20; i++) {
    printf("d %d\n", (int) d2->generate());
  }

  KeyGenerator kg(g);
  */

  // Generator *g = createFacebookValue();
  // //  Generator *g = createGenerator("pareto:15,214.476,0.348238");
  // for (int i = 0; i < 1000000; i++)
  //   printf("%f\n", g->generate());

  int records = 400000000;
  // for (int seed = 1; seed < 100; seed++) {
  //         Generator *g = new Zipf(records, 0.99, seed);
  //         for (int i = 0; i < records; i++)
  //       	  g->generate();
  //         delete g;
  // }
  // return 0;


  Generator *valuesize = createFacebookValue();
  Generator *keysize = createFacebookKey();
  KeyGenerator *keygen = new KeyGenerator(keysize, records);
  FILE *trace;
  trace = fopen("/opt/memcached/trace", "wb");

  for (int i = 0; i < records; i++) {
    std::string keystr = keygen->generate(i);
    int val_len = valuesize->generate();
    // int index = lrand48() % (1024 * 1024);
    // char *val = (char *) malloc(val_len);
    struct trace_data td = {
	    .key_num = i,
	    .key_sz = keystr.length(),
	    .val_sz = val_len,
    };
    fwrite(&td, sizeof(struct trace_data), 1, trace);
    if (i % 10000000 == 0)
	    printf("%d/%d\n", i,records);
    // snprintf(val, val_len, &random_char[index]);
    // printf("%d%d\n", keystr.c_str(), val);
  }

  /*
  Generator *p2 = createGenerator("p:214.476,0.348238");
  //  for (int i = 0; i < 1000; i++)
  //    printf("%f\n", p2->generate());

  p2->set_lambda(1000);
  for (int i = 0; i < 1000; i++)
    printf("%f\n", p2->generate());
  */

  //  for (int i = 0; i < 10000; i++)
  //    printf("%s\n", kg.generate(i).c_str());

  /*
  for (uint64_t ind = 0; ind < 10000; ind++) {
    //  uint64_t ind = 0;
    uint64_t h = fnv_64(ind);
    double U = (double) h / ULLONG_MAX;
  //  double E = e->generate(U); // -log(U);
    double G = g->generate(U);
    int keylen = MAX(round(G), floor(log10(10000)) + 1);

    //    printf("ind=%" PRIu64 "\n", ind);
    //    printf("h=%" PRIu64 "\n", h);
    //    printf("U=%f\n", U);
    //    printf("G=%f\n", G);
    //    printf("keylen=%d\n", keylen);
    printf("%7" PRIu64 " %7d key=%0*" PRIu64 "\n", ind, keylen, keylen, ind);
  }
  */
}
