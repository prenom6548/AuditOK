// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use rstar::{PointDistance, RTree, RTreeObject, AABB};

fn geo_distance2_approx(lat1: f32, lon1: f32, lat2: f32, lon2: f32) -> f32 {
    // Equirectangular approximation: more stable than raw lat/lon Euclidean distance
    // for nearest-neighbor on spherical coordinates.
    let to_rad = std::f32::consts::PI / 180.0;
    let x = (lon2 - lon1) * to_rad * ((lat1 + lat2) * 0.5 * to_rad).cos();
    let y = (lat2 - lat1) * to_rad;
    x * x + y * y
}

pub trait SpatialBounds {
    fn contains(&self, lat: f32, lon: f32) -> bool;
    fn bounds(&self) -> (f32, f32, f32, f32);
}

#[derive(Clone)]
struct SpatialItem<T: SpatialBounds, U> {
    bounds: (f32, f32, f32, f32),
    data: U,
    _marker: std::marker::PhantomData<T>,
}

impl<T: SpatialBounds, U> RTreeObject for SpatialItem<T, U> {
    type Envelope = AABB<[f32; 2]>;

    fn envelope(&self) -> Self::Envelope {
        let (min_lat, min_lon, max_lat, max_lon) = self.bounds;
        AABB::from_corners([min_lat, min_lon], [max_lat, max_lon])
    }
}

impl<T: SpatialBounds, U> PointDistance for SpatialItem<T, U> {
    fn distance_2(&self, point: &[f32; 2]) -> f32 {
        let (min_lat, min_lon, max_lat, max_lon) = self.bounds;
        let center_lat = (min_lat + max_lat) * 0.5;
        let center_lon = (min_lon + max_lon) * 0.5;
        geo_distance2_approx(center_lat, center_lon, point[0], point[1])
    }
}

#[derive(Clone)]
pub struct SpatialIndex<T: SpatialBounds, U> {
    tree: RTree<SpatialItem<T, U>>,
}

impl<T: SpatialBounds, U> SpatialIndex<T, U> {
    pub fn new() -> Self {
        SpatialIndex { tree: RTree::new() }
    }

    pub fn insert(&mut self, bounds: T, data: U) {
        let b = bounds.bounds();
        let item = SpatialItem {
            bounds: b,
            data,
            _marker: std::marker::PhantomData,
        };
        self.tree.insert(item);
    }

    pub fn query(&self, lat: f32, lon: f32) -> Vec<&U> {
        let point = [lat, lon];
        self.tree
            .locate_all_at_point(&point)
            .map(|item| &item.data)
            .collect()
    }

    /// 返回给定坐标最近的一个对象。
    pub fn nearest(&self, lat: f32, lon: f32) -> Option<&U> {
        let point = [lat, lon];
        self.tree
            .nearest_neighbor(&point)
            .map(|item| &item.data)
    }
}

