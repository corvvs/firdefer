#include "fdf.h"

/*
 * set t as an isometric projection (for my standard)
 */
void	ff_set_tr_isometric(t_transform *t)
{
	ft_bzero(t, sizeof(t_transform));
	t->xx = +1 / sqrt(2);
	t->xy = -1 / sqrt(2);
	t->yx = +1 / sqrt(6);
	t->yy = +1 / sqrt(6);
	t->yz = +2 / sqrt(6);
	t->zx = -1 / sqrt(3);
	t->zy = -1 / sqrt(3);
	t->zz = +1 / sqrt(3);
	t->tt = 1;
}

/*
 * set t as an translate transform
 */
void	ff_set_tr_translate(t_transform *t, double x, double y, double z)
{
	ft_bzero(t, sizeof(t_transform));
	t->xx = 1;
	t->yy = 1;
	t->zz = 1;
	t->xt = x;
	t->yt = y;
	t->zt = z;
	t->tt = 1;
}

/*
 * set t as an scaling transform
 */
void	ff_set_tr_scale(t_transform *t, double mx, double my, double mz)
{
	ft_bzero(t, sizeof(t_transform));
	t->xx = mx;
	t->yy = my;
	t->zz = mz;
	t->tt = 1;
}

/*
 * copy s into t
 */
void	ff_tr_copy(t_transform *t, t_transform *s)
{
	ft_memcpy(t, s, sizeof(t_transform));
}

/*
 * compose transforms t and s, and set the result to r.
 */
void	ff_tr_compose(t_transform *t, t_transform *s, t_transform *r)
{
	t_transform u;

	u.xx = t->xx * s->xx + t->xy * s->yx + t->xz * s->zx + t->xt * s->tx;
	u.xy = t->xx * s->xy + t->xy * s->yy + t->xz * s->zy + t->xt * s->ty;
	u.xz = t->xx * s->xz + t->xy * s->yz + t->xz * s->zz + t->xt * s->tz;
	u.xt = t->xx * s->xt + t->xy * s->yt + t->xz * s->zt + t->xt * s->tt;
	u.yx = t->yx * s->xx + t->yy * s->yx + t->yz * s->zx + t->yt * s->tx;
	u.yy = t->yx * s->xy + t->yy * s->yy + t->yz * s->zy + t->yt * s->ty;
	u.yz = t->yx * s->xz + t->yy * s->yz + t->yz * s->zz + t->yt * s->tz;
	u.yt = t->yx * s->xt + t->yy * s->yt + t->yz * s->zt + t->yt * s->tt;
	u.zx = t->zx * s->xx + t->zy * s->yx + t->zz * s->zx + t->zt * s->tx;
	u.zy = t->zx * s->xy + t->zy * s->yy + t->zz * s->zy + t->zt * s->ty;
	u.zz = t->zx * s->xz + t->zy * s->yz + t->zz * s->zz + t->zt * s->tz;
	u.zt = t->zx * s->xt + t->zy * s->yt + t->zz * s->zt + t->zt * s->tt;
	u.tx = t->tx * s->xx + t->ty * s->yx + t->tz * s->zx + t->tt * s->tx;
	u.ty = t->tx * s->xy + t->ty * s->yy + t->tz * s->zy + t->tt * s->ty;
	u.tz = t->tx * s->xz + t->ty * s->yz + t->tz * s->zz + t->tt * s->tz;
	u.tt = t->tx * s->xt + t->ty * s->yt + t->tz * s->zt + t->tt * s->tt;
	ff_tr_copy(r, &u);
	printf("%+.2f %+.2f %+.2f %+.2f\n", r->xx, r->xy, r->xz, r->xt);
	printf("%+.2f %+.2f %+.2f %+.2f\n", r->yx, r->yy, r->yz, r->yt);
	printf("%+.2f %+.2f %+.2f %+.2f\n", r->zx, r->zy, r->zz, r->zt);
	printf("%+.2f %+.2f %+.2f %+.2f\n", r->tx, r->ty, r->tz, r->tt);
}

void	ff_transform_point(t_transform *transform, t_mappoint *point)
{
	t_transform	*f;
	t_mappoint	*p;

	f = transform;
	p = point;
	p->vx = f->xx * p->x + f->xy * p->y + f->xz * p->z + f->xt;
	p->vy = f->yx * p->x + f->yy * p->y + f->yz * p->z + f->yt;
	p->vz = f->zx * p->x + f->zy * p->y + f->zz * p->z + f->zt;
}

void	ff_enclosing_transform(t_master *master)
{
	int			i;

	master->vxmin = +1E300;
	master->vymin = +1E300;
	master->vzmin = +1E300;
	master->vxmax = -1E300;
	master->vymax = -1E300;
	master->vzmax = -1E300;
	i = -1;
	while (++i < (int)master->points_used)
	{
		if (master->vxmin > master->points[i]->vx)
			master->vxmin = master->points[i]->vx;
		if (master->vxmax < master->points[i]->vx)
			master->vxmax = master->points[i]->vx;
		if (master->vymin > master->points[i]->vy)
			master->vymin = master->points[i]->vy;
		if (master->vymax < master->points[i]->vy)
			master->vymax = master->points[i]->vy;
		if (master->vzmin > master->points[i]->vz)
			master->vzmin = master->points[i]->vz;
		if (master->vzmax < master->points[i]->vz)
			master->vzmax = master->points[i]->vz;
	}
}

void	ff_default_transform(t_master *master)
{
	t_transform	t;
	double 		mag;

	// V -> M
	ff_apply_transform(master);
	// MBR(最小概説矩形)を求める
	ff_enclosing_transform(master);
	// MBRの中心を原点に合わせる
	ff_set_tr_translate(&t, -(master->vxmin + master->vxmax) / 2, -(master->vymin + master->vymax) / 2, 0);
	ff_tr_compose(&t, &(master->transform), &(master->transform));
	// 画面の中心を原点に合わせる
	ff_set_tr_translate(&t, -(double)master->window_width / 2, -(double)master->window_height / 2, 0);
	if (master->points_used > 1)
	{
		// MBRが画面内にぴったりおさまるようズーム
		mag = master->window_width / (master->vxmax - master->vxmin);
		if (mag > master->window_height / (master->vymax - master->vymin))
			mag = master->window_height / (master->vymax - master->vymin);
		ff_set_tr_scale(&t, mag, mag, mag);
		ff_tr_compose(&t, &(master->transform), &(master->transform));
	}
	// 画面の中心を戻す
	ff_set_tr_translate(&t, (double)master->window_width / 2, (double)master->window_height / 2, 0);
	ff_tr_compose(&t, &(master->transform), &(master->transform));
	// M系のY軸を反転
	ff_set_tr_scale(&t, 1, -1, 1);
	ff_tr_compose(&t, &(master->transform), &(master->transform));
	ff_set_tr_translate(&t, 0, 1 * master->window_height, 0);
	ff_tr_compose(&t, &(master->transform), &(master->transform));
}