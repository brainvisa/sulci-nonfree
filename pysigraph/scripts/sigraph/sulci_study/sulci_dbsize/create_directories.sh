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
SIERROR='echo -en siErrorLightWrapper.py -t $TRANSLATION -l $f -b $g -c test_global.csv -n $n; echo -en ";" siError.py -l $f -b $g -s $n --csv test_local.csv'

mkdir dbsize_$1;
cd dbsize_$1;
for side in Left Right;
do
	echo "- generate database for $side side...";

	# generate training/testing splits
	#mkdir base_$side;
	#cd base_$side
	#$RANDDB -n $1 -r $2 -f ../../all_${side}_graphs.dat
	#cd ..

	echo "  copy database for several models...";
	# copy
	#for dir in nodes_prior gaussian_model spam_model registred_spam_model locally_from_global_registred_spam_model;
	#do
	#	cp -a base_$side ${dir}_$side;
	#done;

	echo "  - prior model";
	# prior / learning
	cd nodes_prior_$side
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/learn_priors.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --type label_frequency\" > ${d}_learning.log"; done > batch_learning
	cd ..

	echo "  - gaussian model";
	# gaussian / learning
	cd gaussian_model_$side
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/create_segment_databases.py -t $TRANSLATION --node-model-type refgravity_center $(cat $d/train_graphs.dat) && python $CARTOPACK/scripts/sigraph/learn_gaussians_distributions.py \" > ${d}_learning.log"; done > batch_learning
	cd ..

	echo "  - spam (Talairach) model";
	cd spam_model_$side
	# Talairach / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/learn_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_learning
	# Talairach / testing
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/independent_tag.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $f -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat; $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	cd ..

	echo "  - spam (global rigid registration) model";
	cd registred_spam_model_$side
	# Global registration / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_learning
	# Global registration / testing
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/independent_tag_with_registration.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $(echo $g | sed 's#.*/##g;s#.arg#_auto.arg#g') -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat --motion motion_${n}.trm --mode global; $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	cd ..

	echo "  - spam (local rigid registration) model";
	cd locally_from_global_registred_spam_model_$side
	# Local registration / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py --translation $TRANSLATION --sigma-value 2 --mode local --distrib-gaussians gravity_centers $(cat $d/train_graphs.dat) == $(for a in $(cat $d/train_graphs.dat); do b=$(basename $a); echo -en ../../registred_spam_model_${side}/$d/bayesian_spam_distribs/${b%%.arg}_motion.trm' ' ; done) && python $CARTOPACK/scripts/sigraph/sulci_registration/learn_transformation_prior.py *motion_local.dat\" > ${d}_learning.log"; done > batch_learning2
	# Local registration / testing
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python $CARTOPACK/scripts/sigraph/sulci_registration/independent_tag_with_registration.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $(echo $g | sed 's#.*/##g;s#.arg#_auto.arg#g') -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat --motion motion_${n}.trm --mode local --translation-prior bayesian_translation_distribs.dat --direction-prior bayesian_direction_distribs.dat --angle-prior bayesian_angle_distribs.dat --distrib-gaussians ../../gaussian_model_${side}/$d/bayesian_gaussian_distribs.dat --input-motion ../../registred_spam_model_${side}/$d/motion_${n}.trm; $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	cd ..
	cat *_$side/batch_learning >> batch_learning
	cat locally_from_global_registred_spam_model_$side/batch_learning2 >> batch_learning2
	cat *_$side/batch_testing >> batch_testing

	# cleaning
	rm -rf base_$side
done;

cd ..
