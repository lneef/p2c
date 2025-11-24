#include "internal.h"

int main(int argc, char* argv[]) {
   // ------------------------------------------------------------
   // TPC-H Query 5; should return the following on sf1 according to umbra:
   // INDONESIA 55502041.1697
   // VIETNAM 55295086.9967
   // CHINA 53724494.2566
   // INDIA 52035512.0002
   // JAPAN 45410175.6954
   // ------------------------------------------------------------
   // select
   //       n_name,
   //       sum(l_extendedprice * (1 - l_discount)) as revenue
   // from
   //       customer,
   //       orders,
   //       lineitem,
   //       supplier,
   //       nation,
   //       region
   // where
   //       c_custkey = o_custkey
   //       and l_orderkey = o_orderkey
   //       and l_suppkey = s_suppkey
   //       and c_nationkey = s_nationkey
   //       and s_nationkey = n_nationkey
   //       and n_regionkey = r_regionkey
   //       and r_name = 'ASIA'
   //       and o_orderdate >= date '1994-01-01'
   //       and o_orderdate < date '1994-01-01' + interval '1' year
   // group by
   //       n_name
   // order by
   //       revenue desc
   // ------------------------------------------------------------
   {
      auto r = make_unique<Scan>("region");
      IU* r_regionkey = r->getIU("r_regionkey");
      IU* r_name = r->getIU("r_name");
      auto r_sel =
          make_unique<Selection>(std::move(r), makeCallExp("std::equal_to()", make_unique<IUExp>(r_name),
                                                           make_unique<ConstExp<StringView>>("ASIA")));

      auto n = make_unique<Scan>("nation");
      IU* n_nationkey = n->getIU("n_nationkey");
      IU* n_regionkey = n->getIU("n_regionkey");
      IU* n_name = n->getIU("n_name");
      auto join1 = make_unique<InnerJoin>(std::move(r_sel), std::move(n), vector<IU*>{r_regionkey}, vector<IU*>{n_regionkey}, nullptr);

      auto c = make_unique<Scan>("customer");
      IU* c_custkey = c->getIU("c_custkey");
      IU* c_nationkey = c->getIU("c_nationkey");
      auto join2 = make_unique<InnerJoin>(std::move(join1), std::move(c), vector<IU*>{n_nationkey}, vector<IU*>{c_nationkey}, nullptr);

      auto o = make_unique<Scan>("orders");
      auto o_orderkey = o->getIU("o_orderkey");
      auto o_custkey = o->getIU("o_custkey");
      auto o_orderdate = o->getIU("o_orderdate");
      auto lowerBoundExp = makeCallExp("std::greater_equal()", make_unique<IUExp>(o_orderdate), make_unique<ConstExp<int32_t>>(2449354));
      auto upperBoundExp = makeCallExp("std::less_equal()", make_unique<IUExp>(o_orderdate), make_unique<ConstExp<int32_t>>(2449718));
      auto o_sel = make_unique<Selection>(std::move(o), makeCallExp("std::logical_and()", std::move(lowerBoundExp), std::move(upperBoundExp)));
      auto join3 = make_unique<InnerJoin>(std::move(join2), std::move(o_sel), vector<IU*>{c_custkey}, vector<IU*>{o_custkey}, nullptr);

      auto l = make_unique<Scan>("lineitem");
      auto l_orderkey = l->getIU("l_orderkey");
      auto l_suppkey = l->getIU("l_suppkey");
      auto l_extendedprice = l->getIU("l_extendedprice");
      auto l_discount = l->getIU("l_discount");
      auto join4 = make_unique<InnerJoin>(std::move(join3), std::move(l), vector<IU*>{o_orderkey}, vector<IU*>{l_orderkey}, nullptr);

      auto s = make_unique<Scan>("supplier");
      auto s_suppkey = s->getIU("s_suppkey");
      auto s_nationkey = s->getIU("s_nationkey");
      auto join5 = make_unique<InnerJoin>(std::move(s), std::move(join4), vector<IU*>{s_suppkey, s_nationkey}, vector<IU*>{l_suppkey, n_nationkey}, nullptr);

      auto discountPriceExp = makeCallExp("std::multiplies()", make_unique<IUExp>(l_extendedprice), makeCallExp("std::minus()", make_unique<ConstExp<double>>(1.0), make_unique<IUExp>(l_discount)));
      auto discountPriceMap = make_unique<Map>(std::move(join5), std::move(discountPriceExp), "revenue", TypeEnum::Double);
      auto discountPrice = discountPriceMap->getIU("revenue");

      auto gb = make_unique<Aggregation>(std::move(discountPriceMap), IUSet({n_name}));
      gb->addAggregate(make_unique<SumAggregate>("revenue", discountPrice));
      auto revenue = gb->getIU("revenue");

      auto sort = make_unique<Sort>(std::move(gb), vector<IU*>{revenue}, vector<bool>{true});
      produce(std::move(sort), std::vector<IU*>{n_name, revenue}, std::vector<std::string>{"nation","o_year","sum_profit"}, make_unique<PrintTupleSink>());
   }
   return 0;
}
