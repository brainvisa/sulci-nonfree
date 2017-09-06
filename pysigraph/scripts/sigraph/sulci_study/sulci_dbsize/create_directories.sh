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
size=$1
echo "number of runs: $2"

RANDDB="/home/mp210984/svn/brainvisa/sulci/sulci-private/trunk/pysigraph/scripts/sigraph/sulci_misc/randdb.py"
CV="/home/mp210984/svn/brainvisa/sulci/sulci-private/trunk/pysigraph/scripts/sigraph/sulci_misc/cv.py"
CARTOPACK="/i2bm/research/Mandriva-2008.0-i686/cartopack-Mandriva-2008.0-i686-devel_4.0-2009_07_28i2bm"
TRANSLATION="$CARTOPACK/share/shfj-4.0/nomenclature/translation/sulci_model_2008.trl"
SETUP=". $CARTOPACK/bin/cartopack.sh $CARTOPACK/"
SIERROR='echo -en siErrorLightWrapper.py -t $TRANSLATION -l $f -b $g -c test_global.csv -n $n; echo -en "&&" siError.py -t $TRANSLATION -l $f -b $g -s $n --csv test_local.csv'

mkdir dbsize_$1;
cd dbsize_$1;
for side in Left Right;
do
	echo "- generate database for $side side...";

	# generate training/testing splits
	mkdir base_$side;
	cd base_$side
	total_size=$(cat ../../all_${side}_graphs.dat| wc -l)
	if [ "${total_size}" -eq "$1" ];
	then
		$CV -n $1 -f ../../all_${side}_graphs.dat;
		for d in cv_*; do mv $d $(echo $d | sed 's/cv_/run_/g'); done;
	elif [ "1" -eq "$1" ];
	then
		$CV -n ${total_size} -f ../../all_${side}_graphs.dat
		for d in cv_*; do
			mv $d/train_graphs.dat $d/tmp.dat;
			mv -f $d/test_graphs.dat $d/train_graphs.dat;
			mv $d/tmp.dat $d/test_graphs.dat;
			mv $d $(echo $d | sed 's/cv_/run_/g');
		done;
	else
		$RANDDB -n $1 -r $2 -f ../../all_${side}_graphs.dat
	fi;
	cd ..

	echo "  copy database for several models...";
	# copy
	for dir in nodes_prior spam_model registred_spam_model locally_from_global_registred_spam_model;
	do
		cp -a base_$side ${dir}_$side;
	done;

	echo "  - prior model";
	# prior / learning
	cd nodes_prior_$side
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/learn_priors.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --type label_frequency\" > ${d}_learning.log"; done > batch_learning
	cd ..

	#echo "  - gaussian model";
	# gaussian / learning
	#cd gaussian_model_$side
	#for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/create_segment_databases.py -t $TRANSLATION --node-model-type refgravity_center $(cat $d/train_graphs.dat) && python2 $CARTOPACK/scripts/sigraph/learn_gaussians_distributions.py \" > ${d}_learning.log"; done > batch_learning
	#cd ..

	echo "  - spam (Talairach) model";
	cd spam_model_$side
	# Talairach / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/learn_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_learning
	# Talairach / testing
	# with old script
	#for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/independent_tag.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $f -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat && $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/sulci_registration/independent_tag_with_registration.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $f -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat --motion motion_${n}.trm --mode global --maxiter 0 && $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	cd ..

	echo "  - spam (global rigid registration) model";
	cd registred_spam_model_$side
	# Global registration / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py $(cat $d/train_graphs.dat) --translation $TRANSLATION --sigma-value 2\" > ${d}_learning.log"; done > batch_learning
	# Global registration / testing
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/sulci_registration/independent_tag_with_registration.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $f -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat --motion motion_${n}.trm --mode global && $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing
	cd ..

	echo "  - spam (local rigid registration) model";
	cd locally_from_global_registred_spam_model_$side
	# Local registration / learning
	for d in run_*; do echo "cd $PWD/$d && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/sulci_registration/learn_registred_spams_distributions.py --translation $TRANSLATION --sigma-value 2 --mode local --distrib-gaussians gravity_centers $(cat $d/train_graphs.dat) == $(for a in $(cat $d/train_graphs.dat); do b=$(basename $a); echo -en ../../registred_spam_model_${side}/$d/bayesian_spam_distribs/${b%%.arg}_motion.trm' ' ; done) && python2 $CARTOPACK/scripts/sigraph/sulci_registration/learn_transformation_prior.py *motion_local.dat\" > ${d}_learning.log"; done > batch_learning2
	# Local registration / testing
	# use gravity_centers (mean sulcu location in destination space)
	for d in run_*; do cd $d; for g in $(cat test_graphs.dat); do n=$(echo $(basename $g) | sed 's/^[LR]//g;s/_default_session_manual.arg//g;s/_man.arg//g;s/\.arg//g'); f=$(echo $g | sed 's#.*/##g;s#manual#auto#g;s#_man\.#_auto\.#g'); echo $f | grep auto > /dev/null; if [ $? -eq 1 ]; then f=$(echo $f| sed 's#\.arg#_auto\.arg#g'); fi; echo "cd $PWD && nice bash -c \"$SETUP; python2 $CARTOPACK/scripts/sigraph/sulci_registration/independent_tag_with_registration.py -t $TRANSLATION -d bayesian_spam_distribs.dat -i $g -o $f -p ../../nodes_prior_${side}/$d/bayesian_prior.dat -c posteriors_${n}.csv -l output_labels_${n}.dat --motion motion_${n}.trm --mode local --translation-prior bayesian_translation_distribs.dat --direction-prior bayesian_direction_distribs.dat --angle-prior bayesian_angle_distribs.dat --distrib-gaussians gravity_centers.dat --input-motion ../../registred_spam_model_${side}/$d/motion_${n}.trm && $(eval $SIERROR)\" > ${d}_${n}_testing.log"; done; cd ..; done > batch_testing2
	cd ..
	cat *_$side/batch_learning >> batch_learning
	cat locally_from_global_registred_spam_model_$side/batch_learning2 >> batch_learning2
	cat *_$side/batch_testing >> batch_testing
	cat locally_from_global_registred_spam_model_$side/batch_testing2 >> batch_testing2

	# cleaning
	rm -rf base_$side
done;
echo "# learning state 1: prior + Talairach + global rigid registration" > CMD
echo "grid.py --host ~/hosts.Mandriva-2008.0-i686 --tasks batch_learning --timeslot - --log grid_learning.log --broken broken_learning.batch" >> CMD
echo "# learning state 2: local rigid registration" >> CMD
echo "grid.py --host ~/hosts.Mandriva-2008.0-i686 --tasks batch_learning2 --timeslot - --log grid_learning2.log --broken broken_learning2.batch" >> CMD
echo "# testing state 1: Talairach + global rigid registration" >> CMD
echo "grid.py --host ~/hosts.Mandriva-2008.0-i686 --tasks batch_testing --timeslot - --log grid_testing.log --broken broken_testing.batch" >> CMD
echo "# testing state 2: local rigid registration" >> CMD
echo "grid.py --host ~/hosts.Mandriva-2008.0-i686 --tasks batch_testing2 --timeslot - --log grid_testing2.log --broken broken_testing2.batch" >> CMD
echo >> CMD
echo "# check if all results has been computed :" >> CMD
echo "# - local ones :" >> CMD
echo 'for a in *spam*/*/; do n=$(cat $a/test_local.csv 2> /dev/null | awk '\''{ print $1 }'\''| sort| uniq| wc -l); s=$(cat $a/test_graphs.dat| wc -w); let s++; if [ "$n" != "$s" ]; then echo "($n != $s) for '\''$a'\''"; fi; done' >> CMD
echo "# - global ones :" >> CMD
echo 'for a in *spam*/*/; do n=$(cat $a/test_global.csv 2> /dev/null | wc -l); s=$(cat $a/test_graphs.dat| wc -w); let s++; if [ "$n" != "$s" ]; then echo "($n != $s) for '\''$a'\''"; fi; done' >> CMD
echo >> CMD
echo "# create new batch from failed results :" >> CMD
echo "# - local ones :" >> CMD
echo 'for a in *spam*/*/; do n=$(cat $a/test_local.csv 2> /dev/null | awk '\''{ print $1 }'\''| sort| uniq| wc -l); s=$(cat $a/test_graphs.dat| wc -w); let s++; if [ "$n" != "$s" ]; then grep "/$(dirname $a/plop) " batch_testin{g,g2}; fi; done > batch_testing_error' >> CMD
echo "# - global ones :" >> CMD
echo 'for a in *spam*/*/; do n=$(cat $a/test_global.csv 2> /dev/null | wc -l); s=$(cat $a/test_graphs.dat| wc -w); let s++; if [ "$n" != "$s" ]; then grep "/$(dirname $a/plop) " batch_testin{g,g2}; fi; done >> batch_testing_error' >> CMD
echo >> CMD
echo "# get results" >> CMD
echo "# - global ones (left):" >> CMD
echo '/home/mp210984/svn/links/pysigraph/scripts/sigraph/sulci_study/sulci_dbsize/error.py -s '${size}' --output ../results_left_global.csv --mode global locally_from_global_registred_spam_model_Left registred_spam_model_Left spam_model_Left' >> CMD
echo "# - global ones (right):" >> CMD
echo '/home/mp210984/svn/links/pysigraph/scripts/sigraph/sulci_study/sulci_dbsize/error.py -s '${size}' --output ../results_right_global.csv --mode global locally_from_global_registred_spam_model_Right registred_spam_model_Right spam_model_Right' >> CMD
echo "# - local ones (left):" >> CMD
echo '/home/mp210984/svn/links/pysigraph/scripts/sigraph/sulci_study/sulci_dbsize/error.py -s '${size}' --output ../results_left_local.csv --mode local locally_from_global_registred_spam_model_Left registred_spam_model_Left spam_model_Left' >> CMD
echo "# - local ones (right):" >> CMD
echo '/home/mp210984/svn/links/pysigraph/scripts/sigraph/sulci_study/sulci_dbsize/error.py -s '${size}' --output ../results_right_local.csv --mode local locally_from_global_registred_spam_model_Right registred_spam_model_Right spam_model_Right' >> CMD
echo >> CMD
echo "# clean results" >> CMD
echo "/home/mp210984/svn/links/pysigraph/scripts/sigraph/sulci_study/sulci_dbsize/clean.py *spam*/" >> CMD
echo "rm -rf nodes_prior_*" >> CMD
