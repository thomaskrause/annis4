extern crate bencher;
extern crate clap;

extern crate graphannis;

use bencher::Bencher;
use std::path::{Path, PathBuf};
use clap::*;

use bencher::{TDynBenchFn, TestDesc, TestDescAndFn, TestFn, TestOpts};
use std::borrow::Cow;
use std::sync::Arc;

use graphannis::util;
use graphannis::api::corpusstorage::CorpusStorage;

struct CountBench {
    pub def: util::SearchDef,
    pub corpus: String,
    pub cs: Arc<CorpusStorage>,
}

impl TDynBenchFn for CountBench {
    #[allow(unused_must_use)]
    fn run(&self, bench: &mut Bencher) {
        self.cs.preload(&self.corpus);

        bench.iter(|| {
            if let Ok(count) = self.cs.count(&self.corpus, &self.def.json) {
                assert_eq!(self.def.count, count);
            } else {
                assert_eq!(self.def.count, 0);
            }
        });
    }
}

pub fn count_bench(data_dir: &Path, queries_dir: &Path) -> std::vec::Vec<bencher::TestDescAndFn> {
    let mut benches = std::vec::Vec::new();

    let cs = Arc::new(CorpusStorage::new_auto_cache_size(data_dir, false).unwrap());

    // each folder is one corpus
    if let Ok(paths) = std::fs::read_dir(queries_dir) {
        for p in paths {
            if let Ok(p) = p {
                if let Ok(ftype) = p.file_type() {
                    if ftype.is_dir() {
                        if let Ok(corpus_name) = p.file_name().into_string() {
                            let queries = util::get_queries_from_folder(&p.path(), true);
                            for def in queries {
                                let mut bench_name = String::from(corpus_name.clone());
                                bench_name.push_str("/");
                                bench_name.push_str(&def.name);

                                benches.push(TestDescAndFn {
                                    desc: TestDesc {
                                        name: Cow::from(bench_name),
                                        ignore: false,
                                    },
                                    testfn: TestFn::DynBenchFn(Box::new(CountBench {
                                        def,
                                        corpus: corpus_name.clone(),
                                        cs: cs.clone(),
                                    })),
                                });
                            }
                        }
                    }
                }
            }
        }
    }

    return benches;
}

fn main() {
    use bencher::run_tests_console;

    let matches = App::new("graphANNIS search benchmark")
        .arg(Arg::with_name("logfile").long("logfile").takes_value(true))
        .arg(Arg::with_name("data").long("data").short("d").takes_value(true).required(true))
        .arg(Arg::with_name("queries").long("queries").short("q").takes_value(true).required(true))
        .arg(Arg::with_name("FILTER").required(false))
        .get_matches();

    let mut test_opts = TestOpts::default();

    if let Some(filter) = matches.value_of("FILTER") {
        test_opts.filter = Some(String::from(filter));
    }

    if let Some(log) = matches.value_of("logfile") {
        test_opts.logfile = Some(PathBuf::from(log));
    }

    let data_dir: PathBuf = if let Some(dir) = matches.value_of("data") {
        PathBuf::from(dir)
    } else {
        PathBuf::from("data")
    };
    let queries_dir: PathBuf = if let Some(dir) = matches.value_of("queries") {
        PathBuf::from(dir)
    } else {
        PathBuf::from("queries")
    };

    let benches = count_bench(&data_dir, &queries_dir);
    run_tests_console(&test_opts, benches).unwrap();
}
