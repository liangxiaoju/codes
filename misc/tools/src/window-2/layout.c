int find_type(mainwin_t *mainwin, int type)
{
	int i;
	for (i = 0; i < mainwin->box.nsubs; i++) {
		if (mainwin->box.subwin[i].module->type == type) {
			if (mainwin->box.subwin[i].assigned == 0)
				return i;
		}
	}
	return -1;
}

int assign_subwin(mainwin_t *mainwin)
{
	int i, index;
	int igroup = 0;
	boxwin_t *box = &mainwin->box;
	subwin_t *subwin;
	int lines, cols, width, height;

	getmaxyx(mainwin->box.win, lines, cols);

	width = cols / 2;
	height = lines / 2;

	for (i = 0; i < mainwin->box.nsubs; i++) {
		subwin = &box->subwin[i];
		subwin->boxwin = box;
		subwin->id = i;
	}

	for (;;) {
		index = find_type(mainwin, 4);
		if (index == -1)
			break;
		subwin = &box->subwin[index];
		subwin->assigned = 1;
		subwin->win = derwin(box->win, height*2, width*2, 0, 0);
		subwin->group = igroup;
		igroup++;
	}

	for (;;) {
		index = find_type(mainwin, 2);
		if (index == -1)
			break;
		subwin = &box->subwin[index];
		subwin->assigned = 1;
		subwin->win = derwin(box->win, height, width*2, 0, 0);
		subwin->group = igroup;
		if ((index = find_type(mainwin, 2)) != -1) {
			subwin = &box->subwin[index];
			subwin->assigned = 1;
			subwin->win = derwin(box->win, height, width*2, height, 0);
			subwin->group = igroup;
		} else if ((index = find_type(mainwin, 1)) != -1) {
			subwin = &box->subwin[index];
			subwin->assigned = 1;
			subwin->win = derwin(box->win, height, width, height, 0);
			subwin->group = igroup;
			if ((index = find_type(mainwin, 1)) != -1) {
				subwin = &box->subwin[index];
				subwin->assigned = 1;
				subwin->win = derwin(box->win, height, width, height, width);
				subwin->group = igroup;
			}
		}
		igroup++;
	}

	for (;;) {
		index = find_type(mainwin, 3);
		if (index == -1)
			break;
		subwin = &box->subwin[index];
		subwin->assigned = 1;
		subwin->win = derwin(box->win, height*2, width, 0, 0);
		subwin->group = igroup;
		if ((index = find_type(mainwin, 3)) != -1) {
			subwin = &box->subwin[index];
			subwin->assigned = 1;
			subwin->win = derwin(box->win, height*2, width, 0, width);
			subwin->group = igroup;
		} else if ((index = find_type(mainwin, 1)) != -1) {
			subwin = &box->subwin[index];
			subwin->assigned = 1;
			subwin->win = derwin(box->win, height, width, 0, width);
			subwin->group = igroup;
			if ((index = find_type(mainwin, 1)) != -1) {
				subwin = &box->subwin[index];
				subwin->assigned = 1;
				subwin->win = derwin(box->win, height, width, height, width);
				subwin->group = igroup;
			}
		}
		igroup++;
	}

	for (;;) {
		index = find_type(mainwin, 1);
		if (index == -1)
			break;
		for (i = 0; i < 4; i++) {
			index = find_type(mainwin, 1);
			if (index == -1)
				break;
			subwin = &box->subwin[index];
			subwin->assigned = 1;
			subwin->win = derwin(box->win, height, width, i/2*height, i%2*width);
			subwin->group = igroup;
		}
		igroup++;
	}

	return igroup;
}

