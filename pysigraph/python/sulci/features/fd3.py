def sulci_fd3():
	names = [ 'valid', 'extremitiesValid',
		'extremity1x', 'extremity1y', 'extremity1z',
		'extremity2x', 'extremity2y', 'extremity2z',
		'gravityCenter_x', 'gravityCenter_y', 'gravityCenter_z',
		'normalValid', 'normal_x', 'normal_y', 'normal_z',
		'direction_x', 'direction_y', 'direction_z',
		'volume', 'geodesicDepthMax', 'geodesicDepthMin',
		'connectedComponentsAllRels', 'connectedComponents',
		'pureCortical', 'distanceBetweenComponentsMax',
		'plisDePassage', 'hullJunctionsSize', 'surface' ]

	groups = {
		'extremity1' : range(2, 5),
		'extremity2' : range(5, 8),
		'extremities' : range(2, 8),
		'gravity' : range(8, 11),
		'normal' : range(12, 15),
		'direction' : range(15, 18),
		'others' : range(18, 28) }

	hash = dict(zip(range(len(names)), names))
	return {'names' : names, 'groups' : groups, 'indices' : hash}
