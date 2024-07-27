(** Maths and linear algebra.
    Ideally we should avoid doing large amounts of math computation in OCaml,
    and so exposing more functionality for batch processing via Core is preferred *)

(** Base functions that a 2D Vector must support *)
module type Vector = sig
  type t

  val add : t -> t -> t
  val sub : t -> t -> t
  val scalar_mul : float -> t -> t
  val dot : t -> t -> float
end

(** Functor that takes a module implementing the Vector signature and returns
    a module that implements most vector space operations *)
module VectorSpace (V : Vector) = struct
  include V

  let scalar_div s v = scalar_mul (1. /. s) v
  let neg v = scalar_mul (-1.0) v
  let length_squared v = dot v v
  let length v = sqrt (dot v v)
  let magnitude = length

  let normalize v =
    let len = length v in
    if len > 0.0 then scalar_mul (1.0 /. len) v else v

  let distance u v = sub u v |> length
end

module Vec3Float = struct
  type t = { x : float; y : float; z : float }

  let add u v = { x = u.x +. v.x; y = u.y +. v.y; z = u.z +. v.z }
  let sub u v = { x = u.x -. v.x; y = u.y -. v.y; z = u.z -. v.z }
  let scalar_mul k v = { x = k *. v.x; y = k *. v.y; z = k *. v.z }
  let dot u v = (u.x *. v.x) +. (u.y *. v.y) +. (u.z *. v.z)
end

module Vec3 = VectorSpace (Vec3Float)