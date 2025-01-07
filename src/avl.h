#pragma once

#include <map>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <queue>

namespace alp {

  template<typename Pair>
  struct select1st {
      typename Pair::first_type &
      operator()(Pair &x) const { return x.first; }

      const typename Pair::first_type &
      operator()(const Pair &x) const { return x.first; }
  };

  template<typename Pair>
  struct select2nd {
      typename Pair::second_type &
      operator()(Pair &x) const { return x.second; }

      const typename Pair::second_type &
      operator()(const Pair &x) const { return x.second; }
  };

  // Base type for link pointers and height in an AVL node
  struct avl_tree_node_base {
      using base_ptr = avl_tree_node_base *;
      using const_base_ptr = const avl_tree_node_base *;
      using avl_tree_height = int;

      avl_tree_height height_;
      base_ptr parent_;
      base_ptr left_;
      base_ptr right_;

      static base_ptr
      minimum(base_ptr x) noexcept {
          while (x->left_ != nullptr) x = x->left_;
          return x;
      }

      static const_base_ptr
      minimum(const_base_ptr x) noexcept {
          while (x->left_ != nullptr) x = x->left_;
          return x;
      }

      static base_ptr
      maximum(base_ptr x) noexcept {
          while (x->right_ != nullptr) x = x->right_;
          return x;
      }

      static const_base_ptr
      maximum(const_base_ptr x) noexcept {
          while (x->right_ != nullptr) x = x->right_;
          return x;
      }

  };

  template<typename Key_compare_>
  struct avl_tree_key_compare {
      Key_compare_ key_compare_;

      avl_tree_key_compare()
      noexcept(
      std::is_nothrow_default_constructible<Key_compare_>::value)
              : key_compare_() {}

      avl_tree_key_compare(const Key_compare_ &comp)
              : key_compare_(comp) {}

      // Copy constructor added for consistency with C++98 mode.
      avl_tree_key_compare(const avl_tree_key_compare &) = default;

      avl_tree_key_compare(avl_tree_key_compare &&x)
      noexcept(std::is_nothrow_copy_constructible<Key_compare_>::value)
              : key_compare_(x.key_compare_) {}

  };


  // header for the root node of the tree
  struct avl_tree_header {
      // header is a dummy root
      // parent points to real-root node
      // left points to left-most node
      // right points to right-most node

      avl_tree_node_base header_;
      size_t node_count_; // Keeps track of size of tree.

      avl_tree_header() noexcept {
          header_.height_ = 0;
          reset();
      }

      avl_tree_header(avl_tree_header
                      &&x) noexcept {
          if (x.header_.parent_ != nullptr)
              move_data(x);
          else {
              header_.height_ = 0;
              reset();
          }
      }

      void
      move_data(avl_tree_header &from) {
          header_.height_ = from.header_.height_;
          header_.parent_ = from.header_.parent_;
          header_.left_ = from.header_.left_;
          header_.right_ = from.header_.right_;
          header_.parent_->parent_ = &header_;
          node_count_ = from.node_count_;

          from.reset();
      }

      void
      reset() {
          header_.parent_ = nullptr;
          header_.parent_ = nullptr;
          header_.left_ = &header_;
          header_.right_ = &header_;
          node_count_ = 0;
      }
  };


  template<typename Tp_>
  struct aligned_storage_t {
      struct Tp2 { Tp_ t_; };

      alignas(alignof(decltype(Tp2::t_))) std::byte storage_[sizeof(Tp_)];


      aligned_storage_t() = default;

      // Can be used to avoid value-initialization zeroing storage_.
      aligned_storage_t(std::nullptr_t) {}

      void *
      addr() noexcept { return static_cast<void *>(&storage_); }

      const void *
      addr() const noexcept { return static_cast<const void *>(&storage_); }

      Tp_ *
      ptr() noexcept { return static_cast<Tp_ *>(addr()); }

      const Tp_ *
      ptr() const noexcept { return static_cast<const Tp_ *>(addr()); }
  };

  // Node type containing the value and base type
  template<typename Val_>
  struct avl_tree_node : public avl_tree_node_base {
      aligned_storage_t<Val_> storage_;

      Val_ *
      val_ptr() { return storage_.ptr(); }

      const Val_ *
      val_ptr() const { return storage_.ptr(); }


  };

  #if defined(GNUC) || defined(clang)
#define PURE attribute((pure))
  #else
#define PURE
  #endif

  PURE avl_tree_node_base *
  avl_tree_increment(avl_tree_node_base *x)
  noexcept {
      if (x->right_ != nullptr) {
          x = x->right_;
          while (x->left_ != nullptr)
              x = x->left_;
      } else {
          avl_tree_node_base *y = x->parent_;
          while (x == y->right_) {
              x = y;
              y = y->parent_;
          }
          if (x->right_ != y)
              x = y;
      }
      return x;
  }

  PURE const avl_tree_node_base *
  avl_tree_increment(const avl_tree_node_base *x) noexcept {
      return avl_tree_increment(const_cast<avl_tree_node_base *>(x));
  }

  PURE avl_tree_node_base *
  avl_tree_decrement(avl_tree_node_base *x) noexcept {
      if (x->parent_ == x) {
          x = x->right_;
      } else if (x->left_ != nullptr) {
          x = x->left_;
          return avl_tree_node_base::maximum(x);
      } else {
          avl_tree_node_base *y = x->parent_;
          while (x == y->left_) {
              x = y;
              y = y->parent_;
          }
          x = y;
      }
      return x;
  }

  PURE const avl_tree_node_base *
  avl_tree_decrement(const avl_tree_node_base *x) noexcept {
      return avl_tree_decrement(const_cast<avl_tree_node_base *>(x));
  }


  template<typename Tp_>
  struct avl_tree_iterator {
      using vaue_type = Tp_;
      using reference = Tp_ &;
      using pointer = Tp_ *;

      using iterator_category = std::bidirectional_iterator_tag;

      using difference_type = std::ptrdiff_t;
      using self = avl_tree_iterator<Tp_>;
      using base_ptr = avl_tree_node_base::base_ptr;
      using link_type = avl_tree_node<Tp_> *;


      avl_tree_iterator() noexcept
              : node_() {}

      explicit avl_tree_iterator(base_ptr x) noexcept
              : node_(x) {}

      reference
      operator*() const noexcept { return *static_cast<link_type>(node_)->val_ptr(); }

      pointer
      operator->() const noexcept { return static_cast<link_type> (node_)->val_ptr(); }

      self &
      operator++() noexcept {
          node_ = avl_tree_increment(node_);
          return *this;
      }

      self
      operator++(int) noexcept {
          self tmp = *this;
          node_ = avl_tree_increment(node_);
          return tmp;
      }

      self &
      operator--() noexcept {
          node_ = avl_tree_decrement(node_);
          return *this;
      }

      self
      operator--(int) noexcept {
          self tmp = *this;
          node_ = avl_tree_decrement(node_);
          return tmp;
      }

      friend bool
      operator==(const self &x, const self &y) noexcept { return x.node_ == y.node_; }

      friend bool
      operator!=(const self &x, const self &y) noexcept { return x.node_ != y.node_; }

      base_ptr node_;
  };

  template<typename Tp_>
  struct avl_tree_const_iterator {
      using value_type = Tp_;
      using reference = const Tp_ &;
      using pointer = const Tp_ *;

      using iterator = avl_tree_iterator<Tp_>;
      using iterator_category = std::bidirectional_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using self = avl_tree_const_iterator<Tp_>;
      using base_ptr = avl_tree_node_base::const_base_ptr;
      using link_type = const avl_tree_node<Tp_> *;

      avl_tree_const_iterator() noexcept
              : node_() {}

      explicit
      avl_tree_const_iterator(base_ptr
                              x) noexcept
              :
              node_(x) {}

      avl_tree_const_iterator(const iterator &it)
      noexcept
              : node_(it.node_) {}

      iterator
      iter_const_cast() const
      noexcept { return iterator(const_cast<typename iterator::base_ptr>(node_)); }

      reference
      operator*() const
      noexcept { return *static_cast<link_type>(node_)->val_ptr(); }

      pointer
      operator->() const

      noexcept { return static_cast<link_type>(node_)->val_ptr(); }

      self &
      operator++()

      noexcept {
          node_ = avl_tree_increment(node_);
          return *this;
      }

      self
      operator++(int)

      noexcept {
          self tmp = *this;
          node_ = avl_tree_increment(node_);
          return tmp;
      }

      self &
      operator--()

      noexcept {
          node_ = avl_tree_decrement(node_);
          return *this;
      }

      self
      operator--(int)

      noexcept {
          self tmp = *this;
          node_ = avl_tree_decrement(node_);
          return tmp;
      }

      friend bool
      operator==(const self &x, const self &y) noexcept { return x.node_ == y.node_; }

      base_ptr node_;
  };

  struct height_helper {

      static int height(const avl_tree_node_base *x) noexcept {
          return x != nullptr ? x->height_ : 0;
      }

      static bool over_height(const avl_tree_node_base *x) noexcept {
          return std::abs(height_helper::height(x->left_) - height_helper::height(x->right_)) == 2;
      }

      static int new_height(const avl_tree_node_base *x) noexcept {
          return std::max(height(x->left_), height(x->right_)) + 1;
      }
  };

  void avl_rotate_left(avl_tree_node_base *x,
                       avl_tree_node_base *p) noexcept {
      p->right_ = x->left_;
      x->left_ = p;

      x->parent_ = p->parent_;
      p->parent_ = x;

      if (p->right_ != nullptr) {
          p->right_->parent_ = p;
      }

      if (x->parent_ != nullptr) {
          if (x->parent_->parent_ == p)
              x->parent_->parent_ = x;
          else if (x->parent_->left_ == p)
              x->parent_->left_ = x;
          else
              x->parent_->right_ = x;
      }

      p->height_ = height_helper::new_height(p);
      x->height_ = height_helper::new_height(x);
  }


  void avl_rotate_right(avl_tree_node_base *x,
                        avl_tree_node_base *p) noexcept {
      p->left_ = x->right_;
      x->right_ = p;

      x->parent_ = p->parent_;
      p->parent_ = x;


      if (p->left_ != nullptr) {
          p->left_->parent_ = p;
      }

      if (x->parent_ != nullptr) {
          // parent is header
          if (x->parent_->parent_ == p) {
              x->parent_->parent_ = x;
          } else if (x->parent_->right_ == p)
              x->parent_->right_ = x;
          else
              x->parent_->left_ = x;
      }
  }


  enum state : uint32_t {
      right_right = 0,
      left_right = 2,
      right_left = 1,
      left_left = 3
  };

  avl_tree_node_base *avl_tree_rebalance_for_erase(avl_tree_node_base *const z, avl_tree_node_base &header) noexcept {
      avl_tree_node_base *&root = header.parent_;
      avl_tree_node_base *&leftmost = header.left_;
      avl_tree_node_base *&rightmost = header.right_;
      avl_tree_node_base *y = z;
      avl_tree_node_base *x = nullptr;
      avl_tree_node_base *x_parent = nullptr;
      bool erase_left = false;
      int height = 0;
      int tmp_height = 0;

      if (y->left_ == nullptr) { // y has at most one non-null child
          x = y->right_;    // x might be null
      } else if (y->right_ == nullptr) { // y has exactly one non-null child
          x = y->left_;   // x is not null
      } else { // y has two non-null children
          y = y->right_;
          // todo: choose by height
          while (y->left_ != nullptr) {
              y = y->left_;
          }
          x = y->right_;
      }

      if (y != z) { // z
          // relink y in place of z.
          z->left_->parent_ = y;
          y->left_ = z->left_;
          y->height_ = z->height_;

          if (y != z->right_) {
              x_parent = y->parent_;
              if (x) x->parent_ = y->parent_;
              y->parent_->left_ = x;   // y must be a child of left_
              y->right_ = z->right_;
              z->right_->parent_ = y;
              erase_left = true;
          } else {
              erase_left = false;
              x_parent = y;
          }

          if (root == z)
              root = y;
          else if (z->parent_->left_ == z) {
              z->parent_->left_ = y;
          } else {
              z->parent_->right_ = y;
          }
          y->parent_ = z->parent_;

          y = z;
          // y now points to node to be actually deleted
      } else {     // y == z, at most one child
          x_parent = y->parent_;
          if (x)
              x->parent_ = y->parent_;
          if (root == z)
              root = x;
          else if (z->parent_->left_ == z) { // z is a left child
              z->parent_->left_ = x;
              erase_left = true;
          } else {                          // z is a right child
              z->parent_->right_ = x;
              erase_left = false;
          }
          if (leftmost == z) {
              if (z->right_ == nullptr)        // z->left_ must be null also
                  leftmost = z->parent_;
                  // makes leftmost == _M_header if z == root
              else
                  leftmost = avl_tree_node_base::minimum(x);
          }
          if (rightmost == z) {
              if (z->left_ == nullptr)         // z->right_ must be null also
                  rightmost = z->parent_;
                  // makes rightmost == header if z == root
              else                      // x == z->left
                  rightmost = avl_tree_node_base::maximum(x);
          }
      }


      height = height_helper::height(x);
      avl_tree_node_base *tmp = erase_left ? x_parent->right_ : x_parent->left_;
      tmp_height = height_helper::height(tmp);

      int erase_state = 0;
      while (x_parent != &header) {
          int diff = tmp_height - height;
          height = std::max(tmp_height, height) + 1;

          if (height == x_parent->height_ && diff == 1) {
              break;
          }

          // erase_left equals insert right
          erase_state = !erase_left << 1;

          int old_height = x_parent->height_;

          x_parent->height_ = height;

          if (diff == 2) {
              bool is_left;
              int left_height = height_helper::height(tmp->left_);
              int right_height = height_helper::height(tmp->right_);
              if (left_height == right_height) {
                  is_left = !erase_left;
              } else {
                  is_left = left_height > right_height;
              }

              erase_state |= is_left;

              switch (static_cast<state>(erase_state)) {
                  case right_right:
                      avl_rotate_left(tmp, x_parent);
                      break;
                  case left_right:
                      x = is_left ? tmp->left_ : tmp->right_;
                      avl_rotate_left(x, tmp);
                      std::swap(x, tmp);
                      avl_rotate_right(tmp, x_parent);
                      x->height_ = height_helper::new_height(x);
                      break;
                  case left_left:
                      avl_rotate_right(tmp, x_parent);
                      break;
                  case right_left:
                      x = is_left ? tmp->left_ : tmp->right_;
                      avl_rotate_right(x, tmp);
                      std::swap(x, tmp);
                      avl_rotate_left(tmp, x_parent);
                      x->height_ = height_helper::new_height(x);
                      break;
              }
              x_parent->height_ = height_helper::new_height(x_parent);
              tmp->height_ = height_helper::new_height(tmp);

              // tmp is the parent of x and x_parent now
              std::swap(tmp, x_parent);

              if (old_height == x_parent->height_) {
                  break;
              }
          }
          height = x_parent->height_;

          x = x_parent;

          x_parent = x_parent->parent_;

          erase_left = x == x_parent->left_;

          tmp = erase_left ? x_parent->right_ : x_parent->left_;

          tmp_height = height_helper::height(tmp);
      }


      return y;
  }

  void
  avl_tree_insert_and_rebalance(const bool insert_left,
                                avl_tree_node_base *x,
                                avl_tree_node_base *p,
                                avl_tree_node_base &header) noexcept {
      x->height_ = 1;
      x->parent_ = p;
      x->left_ = nullptr;
      x->right_ = nullptr;
      int height = 1;
      int tmp_height = 1;

      if (insert_left) {
          p->left_ = x;
          // insert into an empty tree
          if (p == &header) {
              // pointer the new root
              header.parent_ = x;
              // pointer the right-most element
              header.right_ = x;
              return;
          } else {
              if (p == header.left_) {
                  // maintain header->left pointer left-most element
                  header.left_ = x;
              }
              tmp_height = height_helper::height(p->right_);
          }
      } else {
          p->right_ = x;

          tmp_height = height_helper::height(p->left_);
          if (p == header.right_) {
              header.right_ = x;
          }
      }

      // rebalance the tree
      height = std::max(tmp_height, height) + 1;
      if (height == p->height_) {
          return;
      }

      p->height_ = height;

      auto g = p->parent_;

      uint32_t insert_state = insert_left;

      while (g != &header) {
          bool is_left = g->left_ == p;
          tmp_height = is_left ? height_helper::height(g->right_) : height_helper::height(g->left_);

          int diff = height - tmp_height;

          height = std::max(tmp_height, height) + 1;

          if (height == g->height_) {
              return;
          }

          // is p left child of g?
          insert_state |= is_left << 1;

          g->height_ = height;

          if (diff == 2) {
              switch (static_cast<state>(insert_state)) {
                  case right_right:
                      avl_rotate_left(p, g);
                      break;
                  case left_right:
                      avl_rotate_left(x, p);
                      std::swap(x, p);
                      avl_rotate_right(p, g);
                      x->height_ = height_helper::new_height(x);
                      break;
                  case right_left:
                      avl_rotate_right(x, p);
                      std::swap(x, p);
                      avl_rotate_left(p, g);
                      x->height_ = height_helper::new_height(x);
                      break;
                  case left_left:
                      avl_rotate_right(p, g);
                      break;
              }
              g->height_ = height_helper::new_height(g);
              p->height_ = height_helper::new_height(p);

              // p is the parent of x and g
              // p's height_ == height - 1,
              // so height won't change
              return;
          }
          x = p;
          p = g;
          g = p->parent_;
          insert_state = is_left;
      }

  }

  template<typename T>
  struct is_pair : std::false_type {};

  template<typename T1, typename T2>
  struct is_pair<std::pair<T1, T2>> : std::true_type {};

  template<typename T>
  inline constexpr bool is_pair_v = is_pair<T>::value;


  template<typename K_, typename V_, typename KeyOfValue,
          typename Compare_, typename Alloc_ = std::allocator<V_> >
  class avl_tree {
  public:
      using node_allocator = std::allocator_traits<Alloc_>::template rebind_alloc<avl_tree_node<V_>>;
      using allocator_traits = std::allocator_traits<node_allocator>;

  protected:
      using base_ptr = avl_tree_node_base *;
      using const_base_ptr = const avl_tree_node_base *;
      using link_type = avl_tree_node<V_> *;
      using const_link_type = const avl_tree_node<V_> *;

  public:
      size_t height() const {
          return height_helper::height(this->impl_.header_.parent_);
      }

      Compare_ key_comp() const { return impl_.key_compare_; }

      using key_type = K_;
      using value_type = V_;
      using key_compare = Compare_;

      using pointer = value_type *;
      using const_pointer = const value_type *;
      using reference = value_type &;
      using const_reference = const value_type &;
      using size_type = size_t;
      using difference_type = ptrdiff_t;
      using allocator_type = Alloc_;
      using iterator = avl_tree_iterator<value_type>;
      using const_iterator = avl_tree_const_iterator<value_type>;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      using node_type = typename std::map<K_, std::tuple_element_t<1, V_>, Compare_, Alloc_>::node_type;
      using insert_return_type = typename std::map<K_, std::tuple_element_t<1, V_>, Compare_, Alloc_>::insert_return_type;

      static_assert(is_pair_v<value_type>, "avl_tree must be instantiated with a pair type");
  private:
      void
      destroy_node(link_type p) noexcept {
          allocator_traits::destroy(get_Node_allocator(), p->val_ptr());
          p->~avl_tree_node<V_>();
      }

      void
      drop_node(link_type p) noexcept {
          destroy_node(p);
          put_node(p);
      }

      iterator
      insert_lower_node(base_ptr p, base_ptr z) {
          bool insert_left = (p == tree_end()
                              || !impl_.key_compare_(node_key(p),
                                                     node_key(z)));

          avl_tree_insert_and_rebalance(insert_left, z, p,
                                        this->impl_.header_);
          ++impl_.node_count_;
          return iterator(z);
      }

      iterator
      insert_equal_lower_node(link_type z) {
          link_type x = tree_begin();
          base_ptr y = tree_end();
          while (x != 0) {
              y = x;
              x = !impl_.key_compare_(node_key(x), node_key(z)) ?
                  left(x) : right(x);
          }
          return insert_lower_node(y, z);
      }

      auto
      insert_node(base_ptr x, base_ptr p, link_type z)
      -> iterator {
          bool insert_left = (x != nullptr || p == tree_end()
                              || impl_.key_compare_(node_key(z),
                                                    node_key(p)));

          avl_tree_insert_and_rebalance(insert_left, z, p,
                                        this->impl_.header_);
          ++impl_.node_count_;
          return iterator(z);
      }

      node_allocator &
      get_Node_allocator() noexcept { return this->impl_; }

      const node_allocator &
      get_Node_allocator() const noexcept { return this->impl_; }

      template<typename... Args>
      void
      construct_node(link_type node, Args &&... args) {
          try {
              ::new(node) avl_tree_node<V_>;
              allocator_traits::construct(get_Node_allocator(),
                                          node->val_ptr(),
                                          std::forward<Args>(args)...);
          }
          catch (...) {
              node->~avl_tree_node<V_>();
              put_node(node);
              throw;
          }
      }

      link_type
      get_node() { return allocator_traits::allocate(get_Node_allocator(), 1); }

      void
      put_node(link_type p) noexcept { allocator_traits::deallocate(get_Node_allocator(), p, 1); }

      template<typename... Args>
      link_type
      create_node(Args &&... args) {
          link_type tmp = get_node();
          construct_node(tmp, std::forward<Args>(args)...);
          return tmp;
      }


      struct auto_node {
          template<typename... Args>
          auto_node(avl_tree &t, Args &&... args)
                  : t_(t),
                    node_(t.create_node(std::forward<Args>(args)...)) {}

          ~auto_node() {
              if (node_)
                  t_.drop_node(node_);
          }

          auto_node(auto_node &&n)
                  : t_(n.t_), node_(n.node_) { n.node_ = nullptr; }

          const K_ &
          key() const { return node_key(node_); }

          iterator
          insert(std::pair<base_ptr, base_ptr> p) {
              auto it = t_.insert_node(p.first, p.second, node_);
              node_ = nullptr;
              return it;
          }

          iterator
          insert_equal_lower() {
              auto it = t_.insert_equal_lower_node(node_);
              node_ = nullptr;
              return it;
          }

          avl_tree &t_;
          link_type node_;
      };

      std::pair<base_ptr, base_ptr>
      get_insert_unique_pos(const key_type &k) {
          using res = std::pair<base_ptr, base_ptr>;
          link_type x = tree_begin();
          base_ptr y = tree_end();
          bool comp = true;
          while (x != 0) {
              y = x;
              comp = impl_.key_compare_(k, node_key(x));
              x = comp ? left(x) : right(x);
          }
          iterator j = iterator(y);
          if (comp) {
              if (j == begin())
                  return res(x, y);
              else
                  --j;
          }
          if (impl_.key_compare_(node_key(j.node_), k))
              return res(x, y);
          return res(j.node_, 0);
      }

  public:
  public:
      /// Re-insert an extracted node.
      insert_return_type
      reinsert_node_unique(node_type &&nh) {
          insert_return_type ret;
          if (nh.empty())
              ret.position = end();
          else {
              assert(get_Node_allocator() == nh.get_allocator());

              auto res = get_insert_unique_pos(nh.key());

              if (res.second) {
                  auto_node an(*this, std::move(nh.key()), std::move(nh.mapped()));

                  ret.position = an.insert(res);
                  ret.inserted = true;
              } else {
                  ret.node = std::move(nh);
                  ret.position = iterator(res.first);
                  ret.inserted = false;
              }
          }
          return ret;
      }


      std::pair<base_ptr,
              base_ptr>
      get_insert_equal_pos(const key_type &k) {
          using res = std::pair<base_ptr, base_ptr>;
          link_type x = tree_begin();
          base_ptr y = tree_end();
          while (x != 0) {
              y = x;
              x = impl_.key_compare_(k, node_key(x)) ?
                  left(x) : right(x);
          }
          return res(x, y);
      }

      /// Re-insert an extracted node.
      iterator
      reinsert_node_equal(node_type &&nh) {
          iterator ret;
          if (nh.empty())
              ret = end();
          else {
              assert(get_Node_allocator() == nh.get_allocator());
              auto res = get_insert_equal_pos(nh.key());
              auto_node an(*this, std::move(nh.key()), std::move(nh.mapped()));
              if (res.second)
                  ret = an.insert(res);
              else
                  ret = an.insert_equal_lower();
          }
          return ret;
      }

      /// Re-insert an extracted node.
      iterator
      reinsert_node_hint_unique(const_iterator hint, node_type &&nh) {
          iterator ret;
          if (nh.empty())
              ret = end();
          else {
              assert(get_Node_allocator() == nh.get_allocator());
              auto res = get_insert_hint_unique_pos(hint, nh.key());
              auto_node an(*this, std::move(nh.key()), std::move(nh.mapped()));
              if (res.second) {
                  ret = an.insert(res);
              } else
                  ret = iterator(res.first);
          }
          return ret;
      }

      /// Re-insert an extracted node.
      iterator
      reinsert_node_hint_equal(const_iterator hint, node_type &&nh) {
          iterator ret;
          if (nh.empty())
              ret = end();
          else {
              assert(get_Node_allocator() == nh.get_allocator());
              auto res = get_insert_hint_unique_pos(hint, nh.key());
              auto_node an(*this, std::move(nh.key()), std::move(nh.mapped()));
              if (res.second)
                  ret = an.insert(res);
              else
                  ret = an.insert_equal_lower();

          }
          return ret;
      }


      void check() {
          std::queue<base_ptr> q;
          q.push(root());
          while (!q.empty()) {
              auto x = q.front();
              q.pop();
              if (x == nullptr) {
                  continue;
              }

              auto left_height = height_helper::height(x->left_);
              auto right_height = height_helper::height(x->right_);
              assert(std::abs(left_height - right_height) <= 1);
              assert(x->height_ == std::max(left_height, right_height) + 1);
              q.emplace(x->left_);
              q.emplace(x->right_);
          }
      }


      template<typename ...Args>
      std::pair<iterator, bool> emplace_unique(Args &&...args) {
          auto_node z(*this, std::forward<Args>(args)...);
          auto res = get_insert_unique_pos(z.key());
          if (res.second)
              return {z.insert(res), true};
          return {iterator(res.first), false};
      }

      iterator begin() noexcept { return iterator(this->impl_.header_.left_); }

      const_iterator begin() const noexcept { return const_iterator(this->impl_.header_.left_); }

      iterator
      end() noexcept { return iterator(&this->impl_.header_); }

      const_iterator
      end() const noexcept { return const_iterator(&this->impl_.header_); }

  public:
      node_type extract(const_iterator position) {
          auto res = avl_tree_rebalance_for_erase(position.iter_const_cast().node_, this->impl_.header_);
          --impl_.node_count_;
          return {static_cast<link_type>(res), get_Node_allocator()};
      }

      node_type extract(const key_type &k) {
          node_type nh;
          auto it = find(k);

          if (it == end())
              return nh;
          return extract(const_iterator(it));
      }


  protected:
      template<typename Key_compare_>
      struct avl_tree_impl
              : public node_allocator, public avl_tree_key_compare<Key_compare_>, public avl_tree_header {
          using base_key_compare = avl_tree_key_compare<Key_compare_>;

          avl_tree_impl()
          noexcept(
          std::is_nothrow_default_constructible<node_allocator>::value
          && std::is_nothrow_default_constructible<base_key_compare>::value)
                  : node_allocator() {}

          avl_tree_impl(const avl_tree_impl &x)
                  : node_allocator(allocator_traits::select_on_container_copy_construction(x)),
                    base_key_compare(x.key_compare_),
                    avl_tree_header() {}


          avl_tree_impl(avl_tree_impl &&)
          noexcept(std::is_nothrow_move_constructible<base_key_compare>::value)
          = default;

          explicit
          avl_tree_impl(node_allocator &&a) :
                  node_allocator(std::move(a)) {}

          avl_tree_impl(avl_tree_impl &&x, node_allocator &&a)
                  : node_allocator(std::move(a)),
                    base_key_compare(std::move(x)),
                    avl_tree_header(std::move(x)) {}

          avl_tree_impl(const Key_compare_ &comp, node_allocator &&a)
                  : node_allocator(std::move(a)), base_key_compare(comp) {}

      };


      avl_tree_impl<Compare_> impl_;

  protected:
      base_ptr &
      root() noexcept { return this->impl_.header_.parent_; }

      const_base_ptr
      root() const noexcept { return this->impl_.header_.parent_; }

      base_ptr &
      leftmost() noexcept { return this->impl_.header_.left_; }

      const_base_ptr
      leftmost() const noexcept { return this->impl_.header_.left_; }

      base_ptr &
      rightmost() noexcept { return this->impl_.header_.right_; }

      const_base_ptr
      rightmost() const noexcept { return this->impl_.header_.right_; }

      link_type
      mbegin() const noexcept { return static_cast<link_type>(this->impl_.header_.parent_); }

      link_type
      tree_begin() noexcept { return mbegin(); }

      const_link_type
      tree_begin() const noexcept {
          return static_cast<const_link_type>
          (this->impl_.header_.parent_);
      }

      base_ptr
      tree_end() noexcept { return &this->impl_.header_; }

      const_base_ptr
      tree_end() const noexcept { return &this->impl_.header_; }

      static const K_ &
      node_key(const_link_type x) {
          // If we're asking for the key we're presumably using the comparison
          // object, and so this is a good place to sanity check it.
          static_assert(std::is_invocable<Compare_ &, const K_ &, const K_ &>{},
                        "comparison object must be invocable "
                        "with two arguments of key type");
          // _GLIBCXXresOLVE_LIB_DEFECTS
          // 2542. Missing const requirements for associative containers
          if constexpr (std::is_invocable<Compare_ &, const K_ &, const K_ &>{})
              static_assert(
                      std::is_invocable_v<const Compare_ &, const K_ &, const K_ &>,
                      "comparison object must be invocable as const");

          return KeyOfValue()(*x->val_ptr());
      }

      static link_type
      left(base_ptr x) noexcept { return static_cast<link_type>(x->left_); }

      static const_link_type
      left(const_base_ptr x) noexcept { return static_cast<const_link_type>(x->left_); }

      static link_type
      right(base_ptr x) noexcept { return static_cast<link_type>(x->right_); }

      static const_link_type
      right(const_base_ptr x) noexcept { return static_cast<const_link_type>(x->right_); }

      static const K_ &
      node_key(const_base_ptr x) { return node_key(static_cast<const_link_type>(x)); }

      static base_ptr
      minimum(base_ptr x) noexcept { return avl_tree_node_base::minimum(x); }

      static const_base_ptr
      minimum(const_base_ptr x) noexcept { return avl_tree_node_base::minimum(x); }

      static base_ptr
      maximum(base_ptr x) noexcept { return avl_tree_node_base::maximum(x); }

      static const_base_ptr
      maximum(const_base_ptr x) noexcept { return avl_tree_node_base::maximum(x); }


  public:
      // allocation/deallocation
      avl_tree() = default;

      avl_tree(const Compare_ &comp,
               const allocator_type &a = allocator_type())
              : impl_(comp, node_allocator(a)) {}


      avl_tree(const avl_tree &x)
              : impl_(x.impl_) {
          if (x.root() != 0)
              root() = tree_copy(x);
      }

      avl_tree(avl_tree &&) = default;

      avl_tree(avl_tree &&x, const allocator_type &a)
              : avl_tree(std::move(x), _Node_allocator(a)) {}


      iterator
      find(const K_ &k) {
          iterator j = lower_bound(tree_begin(), tree_end(), k);
          return (j == end()
                  || impl_.key_compare_(k,
                                        node_key(j.node_))) ? end() : j;
      }

      const_iterator
      find(const K_ &k) const {
          const_iterator j = lower_bound(tree_begin(), tree_end(), k);
          return (j == end()
                  || impl_.key_compare_(k,
                                        node_key(j.node_))) ? end() : j;
      }

  public:
      iterator
      lower_bound(link_type x, base_ptr y,
                  const K_ &k) {
          while (x != nullptr)
              if (!impl_.key_compare_(node_key(x), k))
                  y = x, x = left(x);
              else
                  x = right(x);
          return iterator(y);
      }

      const_iterator
      lower_bound(const_link_type x, const_base_ptr y,
                  const K_ &k) const {
          while (x != 0)
              if (!impl_.key_compare_(node_key(x), k))
                  y = x, x = left(x);
              else
                  x = right(x);
          return const_iterator(y);
      }

      iterator
      upper_bound(link_type x, base_ptr y,
                  const K_ &k) {
          while (x != 0)
              if (impl_.key_compare_(k, node_key(x)))
                  y = x, x = left(x);
              else
                  x = right(x);
          return iterator(y);
      }

      const_iterator
      upper_bound(const_link_type x, const_base_ptr y,
                  const K_ &k) const {
          while (x != 0)
              if (impl_.key_compare_(k, node_key(x)))
                  y = x, x = left(x);
              else
                  x = right(x);
          return const_iterator(y);
      }


      iterator
      lower_bound(const key_type &resk) { return lower_bound(tree_begin(), tree_end(), resk); }

      const_iterator
      lower_bound(const key_type &resk) const { return lower_bound(tree_begin(), tree_end(), resk); }

      iterator
      upper_bound(const key_type &resk) { return upper_bound(tree_begin(), tree_end(), resk); }

      const_iterator
      upper_bound(const key_type &resk) const { return upper_bound(tree_begin(), tree_end(), resk); }

      void clear() noexcept {
          tree_erase(tree_begin());
          impl_.reset();
      }

  private:
      void
      tree_erase(link_type x) {
          // destroy the subtree
          // Erase without rebalancing.
          while (x != 0) {
              tree_erase(right(x));
              link_type y = left(x);
              drop_node(x);
              x = y;
          }
      }


      std::pair<iterator, iterator>
      equal_range(const K_ &k) {
          link_type x = tree_begin();
          base_ptr y = tree_end();
          while (x != nullptr) {
              if (impl_.key_compare_(node_key(x), k))
                  // node_key(x) < k
                  x = right(x);
              else if (impl_.key_compare_(k, node_key(x)))
                  // k < node_key(x)
                  y = x, x = left(x);
              else {
                  // k == node_key(x)
                  link_type xu(x);
                  base_ptr yu(y);
                  y = x, x = left(x);
                  xu = right(xu);
                  return std::pair<iterator,
                          iterator>(lower_bound(x, y, k),
                                    upper_bound(xu, yu, k));
              }
          }
          // x == nullptr, k not found, y is
          return std::pair<iterator, iterator>(iterator(y),
                                               iterator(y));
      }

      std::pair<base_ptr, base_ptr>
      get_insert_hint_unique_pos(const_iterator position,
                                 const key_type &k) {
          iterator pos = position.iter_const_cast();
          using res = std::pair<base_ptr, base_ptr>;

          // end()
          if (pos.node_ == tree_end()) {
              if (size() > 0
                  && impl_.key_compare_(node_key(rightmost()), k))
                  return res(0, rightmost());
              else
                  return get_insert_unique_pos(k);
          } else if (impl_.key_compare_(k, node_key(pos.node_))) {
              // First, try before...
              iterator before = pos;
              if (pos.node_ == leftmost()) // begin()
                  return res(leftmost(), leftmost());
              else if (impl_.key_compare_(node_key((--before).node_), k)) {
                  if (right(before.node_) == 0)
                      return res(0, before.node_);
                  else
                      return res(pos.node_, pos.node_);
              } else
                  return get_insert_unique_pos(k);
          } else if (impl_.key_compare_(node_key(pos.node_), k)) {
              // ... then try after.
              iterator after = pos;
              if (pos.node_ == rightmost())
                  return res(0, rightmost());
              else if (impl_.key_compare_(k, node_key((++after).node_))) {
                  if (right(pos.node_) == 0)
                      return res(0, pos.node_);
                  else
                      return res(after.node_, after.node_);
              } else
                  return get_insert_unique_pos(k);
          } else
              // Equivalent keys.
              return res(pos.node_, 0);
      }

      void
      erase_aux(const_iterator position) {
          link_type y =
                  static_cast<link_type>(avl_tree_rebalance_for_erase
                          (const_cast<base_ptr>(position.node_),
                           this->impl_.header_));
          drop_node(y);
          --impl_.node_count_;
      }


      void
      erase_aux(const_iterator first, const_iterator last) {
          if (first == begin() && last == end())
              clear();
          else
              while (first != last)
                  erase_aux(first++);
      }

  public:
      template<typename... Args>
      auto
      emplace_hint_unique(const_iterator pos, Args &&... args)
      -> iterator {
          auto_node z(*this, std::forward<Args>(args)...);
          auto res = get_insert_hint_unique_pos(pos, z.key());
          if (res.second)
              return z.insert(res);
          return iterator(res.first);
      }

      size_t erase(const key_type &k) {
          std::pair<iterator, iterator> p = equal_range(k);
          const size_type old_size = size();
          erase_aux(p.first, p.second);
          return old_size - size();
      }

      size_t size() const noexcept {
          return impl_.node_count_;
      }


      void print() {
          std::cout << "avl_tree: " << std::endl;
          auto x = impl_.header_.parent_;
          std::queue<avl_tree_node_base *> q;
          q.push(x);

          q.push(reinterpret_cast<avl_tree_node_base *>(&impl_));


          while (!q.empty()) {
              auto t = q.front();
              q.pop();
              if (static_cast<void *>(t) == static_cast<void *>(&impl_)) {
                  std::cout << std::endl;
                  continue;
              }

              if (t == nullptr) {
                  std::cout << "null 0";
                  continue;
              }

              std::cout << node_key(t) << " " << t->height_ << " ";

              q.push(t->left_);

              q.push(t->right_);
          }


      }

      ~avl_tree() noexcept {
          tree_erase(tree_begin());
      }

  private:
      struct reuse_or_alloc_node {
          reuse_or_alloc_node(avl_tree &t)
                  : root_(t.root()), nodes_(t.rightmost()), t_(t) {
              if (root_) {
                  root_->parent_ = 0;

                  if (nodes_->left_)
                      nodes_ = nodes_->left_;
              } else
                  nodes_ = nullptr;
          }

          reuse_or_alloc_node(const reuse_or_alloc_node &) = delete;


          ~reuse_or_alloc_node() { t_.tree_erase(static_cast<link_type >(root_)); }

          template<typename Args>
          link_type
          operator()(Args &&arg) {
              link_type node = static_cast<link_type>(extract());
              if (node) {
                  t_.destroy_node(node);
                  t_.construct_node(node, std::forward<Args>(arg));
                  return node;
              }

              return t_.create_node(std::forward<Args>(arg));
          }

      private:
          base_ptr
          extract() {
              if (!nodes_)
                  return nodes_;

              base_ptr node = nodes_;
              nodes_ = nodes_->parent_;
              if (nodes_) {
                  if (nodes_->right_ == node) {
                      nodes_->right_ = nullptr;

                      if (nodes_->left_) {
                          nodes_ = nodes_->left_;

                          while (nodes_->right_)
                              nodes_ = nodes_->right_;

                          if (nodes_->left_)
                              nodes_ = nodes_->left_;
                      }
                  } else // node is on the left.
                      nodes_->left_ = nullptr;
              } else
                  root_ = nullptr;

              return node;
          }

          base_ptr root_;
          base_ptr nodes_;
          avl_tree &t_;
      };

      enum { as_lvalue, as_rvalue };

      template<bool MoveValue, typename NodeGen>
      link_type
      clone_node(link_type x, NodeGen &node_gen) {
          using Vp = std::conditional_t<MoveValue,
                  value_type &&,
                  const value_type &>;
          link_type tmp
                  = node_gen(std::forward<Vp>(*x->val_ptr()));
          tmp->height_ = x->height_;
          tmp->left_ = 0;
          tmp->right_ = 0;
          return tmp;
      }

      struct alloc_node {
          alloc_node(avl_tree &t)
                  : t_(t) {}

          template<typename Arg>
          link_type
          operator()(Arg &&arg) const { return t_.create_node(std::forward<Arg>(arg)); }

      private:
          avl_tree &t_;
      };

      template<bool MoveValues, typename NodeGen>
      link_type tree_copy(const avl_tree &x, NodeGen &gen) {
          link_type root =
                  tree_copy < MoveValues > (x.mbegin(), tree_end(), gen);
          leftmost() = minimum(root);
          rightmost() = maximum(root);
          impl_.node_count_ = x.impl_.node_count_;
          return root;
      }

      link_type
      tree_copy(const avl_tree &x) {
          alloc_node an(*this);
          return tree_copy < as_lvalue > (x, an);
      }

      template<bool MoveValues, typename NodeGen>
      link_type
      tree_copy(link_type x, base_ptr p, NodeGen &node_gen) {
          // Structural copy. x and p must be non-null.
          link_type top = clone_node<MoveValues>(x, node_gen);
          top->parent_ = p;

          try {
              if (x->right_)
                  top->right_ =
                          tree_copy < MoveValues > (right(x), top, node_gen);
              p = top;
              x = left(x);

              while (x != 0) {
                  link_type y = clone_node<MoveValues>(x, node_gen);
                  p->left_ = y;
                  y->parent_ = p;
                  if (x->right_)
                      y->right_ = tree_copy < MoveValues > (right(x),
                              y, node_gen);
                  p = y;
                  x = left(x);
              }
          }
          catch (...) {
              tree_erase(top);
              throw;
          }
          return top;
      }

  public:


      avl_tree &operator=(const avl_tree &x) {
          if (this != std::addressof(x)) {
              // Note that _Key may be a constant type.
              if (allocator_traits::propagate_on_container_copy_assignment()) {
                  auto &this_alloc = this->get_Node_allocator();
                  auto &that_alloc = x.get_Node_allocator();
                  if (!allocator_traits::is_always_equal()
                      && this_alloc != that_alloc) {
                      // Replacement allocator cannot free existing storage, we need
                      // to erase nodes first.
                      clear();
                      std::__alloc_on_copy(this_alloc, that_alloc);
                  }
              }

              reuse_or_alloc_node roan(*this);
              impl_.reset();
              impl_.key_compare_ = x.impl_.key_compare_;
              if (x.root() != 0)
                  root() = tree_copy<as_lvalue>(x, roan);
          }

          return *this;
      }
  };


  template<typename K_, typename Tp_, typename Compare_ = std::less<K_>,
          typename Alloc_ = std::allocator<std::pair<const K_, Tp_> > >
  class avl_map {
  public:
  public:
      using key_type = K_;
      using mapped_type = Tp_;
      using key_compare = Compare_;
      using allocator_type = Alloc_;
      using allocator_traits = std::allocator_traits<allocator_type>;
      using value_type = std::pair<const key_type, mapped_type>;

      using pointer = allocator_traits::pointer;
      using const_pointer = allocator_traits::const_pointer;
      using reference = std::pair<const key_type, mapped_type> &;
      using const_reference = const std::pair<const key_type, mapped_type> &;

      template<typename Up_, typename Vp_ = std::remove_reference_t<Up_>>
      static constexpr bool usable_key = std::disjunction_v<std::is_same<const Vp_, const K_>,
              std::conjunction<std::is_scalar<Vp_>, std::is_scalar<K_>>>;

      using pair_alloc_type = std::allocator_traits<allocator_type>::template rebind_alloc<value_type>;

      typedef avl_tree<key_type, value_type, select1st<value_type>,
              key_compare, pair_alloc_type>
              rep_type;

      using iterator = typename rep_type::iterator;
      using const_iterator = typename rep_type::const_iterator;
      using reverse_iterator = typename rep_type::reverse_iterator;
      using const_reverse_iterator = typename rep_type::const_reverse_iterator;
      using difference_type = typename rep_type::difference_type;
      using size_type = typename rep_type::size_type;


      using node_type = typename rep_type::node_type;
      using insert_return_type = typename rep_type::insert_return_type;


  public:
      size_t height() const noexcept { return t_.height(); }

      avl_map() = default;

      ~avl_map() = default;

      iterator begin() noexcept { return t_.begin(); }

      iterator end() noexcept { return t_.end(); }

      const_iterator begin() const noexcept { return t_.begin(); }

      const_iterator end() const noexcept { return t_.end(); }

      const_iterator
      cbegin() const noexcept { return t_.begin(); }

      const_iterator
      cend() const noexcept { return t_.end(); }

      template<typename ...Args>
      std::pair<iterator, bool>
      emplace(Args &&...args) {
          if constexpr (sizeof...(Args) == 2)
              if constexpr (std::is_same_v<allocator_type, std::allocator<value_type>>) {
                  auto &&[a, v] = std::pair<Args &...>(args...);
                  if constexpr (usable_key < decltype(a) >) {
                      const key_type &k = a;
                      iterator i = lower_bound(k);
                      if (i == end() || key_compare()(k, (*i).first)) {
                          i = emplace_hint(i, std::forward<Args>(args)...);
                          return {i, true};
                      }
                      return {i, false};
                  }
              }
          return t_.emplace_unique(std::forward<Args>(args)...);
      }

      size_t erase(const key_type &k) {
          return t_.erase(k);
      }

      size_t size() const noexcept { return t_.size(); }

      iterator
      lower_bound(const key_type &x) { return t_.lower_bound(x); }

      const_iterator
      lower_bound(const key_type &x) const { return t_.lower_bound(x); }

      iterator
      upper_bound(const key_type &x) { return t_.upper_bound(x); }

      const_iterator
      upper_bound(const key_type &x) const { return t_.upper_bound(x); }

      mapped_type &
      operator[](const key_type &k) requires std::is_default_constructible_v<mapped_type> {
          iterator i = lower_bound(k);
          // i->first is greater than or equivalent to k.
          if (i == end() || key_comp()(k, (*i).first))
              i = t_.emplace_hint_unique(i, std::piecewise_construct,
                                         std::tuple<const key_type &>(k),
                                         std::tuple<>());
          return (*i).second;
      }

      node_type
      extract(const_iterator pos) {
          assert(pos != end());
          return t_.extract(pos);
      }

      /// Extract a node.
      node_type
      extract(const key_type &x) { return t_.extract(x); }

      /// Re-insert an extracted node.
//      insert_return_type
//      insert(node_type &&nh) { return t_.reinsert_node_unique(std::move(nh)); }

//      iterator
//      insert(node_type &&nh) { return t_.reinsert_node_equal(std::move(nh)); }

      /// Re-insert an extracted node.
      iterator
      insert(const_iterator hint, node_type &&nh) { return t_.reinsert_node_hint_unique(hint, std::move(nh)); }


      iterator
      find(const key_type &x) { return t_.find(x); }

      const_iterator
      find(const key_type &x) const { return t_.find(x); }

      void print() {
          t_.print();
      }

      void check() {
          t_.check();
      }

      void clear() noexcept { t_.clear(); }

      avl_map &
      operator=(const avl_map &) = default;

      avl_map(const avl_map &) = default;


  private:
      key_compare key_comp() const { return t_.key_comp(); }

      template<typename... Args>
      iterator
      emplace_hint(const_iterator pos, Args &&... args) {
          return t_.emplace_hint_unique(pos,
                                        std::forward<Args>(args)...);
      }

  private:
      rep_type t_;
  };


} // namespace alp