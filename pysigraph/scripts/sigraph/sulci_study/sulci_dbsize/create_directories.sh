#!/usr/bin/env bash

if [ "$#" -ne "2" ];
then
  echo
  echo "Usage : $(basename $0) size runs"
  echo "  size: training database size."
  echo "  runs: number of generated training/testing databases"
  exit 1
fi

echo "training db size: $1"
echo "number of runs: $2"

RANDDB="/home/mp210984/svn/brainvisa/sulci/sulci-private/trunk/pysigraph/scripts/sigraph/sulci_misc/randdb.py"
CARTOPACK="/i2bm/research/Mandriva-2008.0-i686/cartopack-Mandriva-2008.0-i686-devel_4.0-2009_07_28i2bm"
TRANSLATION="$CARTOPACK/share/shfj-4.0/nomenclature/translation/sulci_model_2008.trl"
SETUP=". $CARTOPACK/bin/cartopack.sh $CARTOPACK/"

mkdir dbsize_$1;
cd dbsize_$1;
for side in Left Right;
do
	echo "- generate database for $side side...";

	# generate training/testing splits
	mkdir base_$side;
	cd base_$side
	$RANDDB -n $1 -r $2 -f ../../all_${side}_graphs.dat
	cd ..

	echo "  copy database for several models...";
	# copy
	for dir in nodes_prior spam_model registred_spam_model locally_from_global_registred_spam_model;
	do
		cp -a base_$side ${dir}_$side;
	done;

	# prior / learning
	cd nodes_prior_$side
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/learn_priors.py -m bayesian_graphmodel.dat $(cat $d/train_graphs.dat) --translation $TRANSLATION --type label_frequency\" > ${d}_learning.log"; done > batch_prior_learning
	cd ..

	
	cd spam_model_$side
	# Talairach / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/learn_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_spam_learning
	# Talairach / testing
	for d in run_*; cd $d; do for g in $(cat test/graphs.dat); do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/independent_tag.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $(echo $g | sed 's#.*/##g;s#.arg#_auto.arg#g') -p ../../nodes_prior_${side}/$d/a/bayesian_prior.dat -c "posterior_independent_prior_nodes_$n.csv" -l "posterior_independent_labels_$n.dat"\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_spam_testing

	cd ..

	cd registred_spam_model_$side
	# Global registration / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_spam_learning
	cd ..

	cd locally_from_global_registred_spam_model_$side
	# Local registration /learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py --translation $TRANSLATION --sigma-value 2 --mode local --distrib-gaussians gravity_centers $(cat $d/train_graphs.dat) == $(for a in $(cat $d/train_graphs.dat); do b=$(basename $a); echo -en ../../registred_spam_model_${side}/$d/bayesian_spam_distribs/${b%%.arg}_motion.trm' ' ; done)\" > ${d}_learning.log"; done > batch_spam_learning
	cd ..
	cat */batch_spam_learning > batch_spam_learning

	# cleaning
	rm -rf base_$side
done;
cd ..
