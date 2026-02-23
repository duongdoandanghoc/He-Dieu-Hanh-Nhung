# Virtual RAM Disk Kernel Module (Hệ Điều Hành Nhúng) 🐧💻

**GVHD:** Phạm Doãn Tĩnh

[![Linux Kernel](https://img.shields.io/badge/Kernel-Linux-FCC624?style=for-the-badge&logo=linux)](https://www.kernel.org/)
[![C](https://img.shields.io/badge/Language-C-A8B9CC?style=for-the-badge&logo=c)](https://en.cppreference.com/w/c)
[![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)](https://github.com/duongdoandanghoc/He-Dieu-Hanh-Nhung)
[![License: GPL](https://img.shields.io/badge/License-GPL-blue.svg?style=for-the-badge)](LICENSE)

## 🌟 Giới thiệu

Dự án này tập trung vào việc phát triển một **Kernel Module** cho hệ điều hành Linux, cụ thể là xây dựng một **Virtual RAM Disk**. Đây là một thiết bị lưu trữ ảo hoạt động trực tiếp trên bộ nhớ RAM, cho phép truy cập dữ liệu với tốc độ cực cao. 

Khi mô-đun được tải, người dùng có thể tương tác với thiết bị thông qua `/dev/asgn1`. Thiết bị tự động quản lý danh sách các trang bộ nhớ, phân bổ thêm dung lượng theo yêu cầu khi thực hiện thao tác ghi. Dự án minh họa các khái niệm cốt lõi trong lập trình hệ thống và hệ điều hành nhúng như quản lý bộ nhớ, xử lý thiết bị ký tự (character devices), và giao tiếp giữa không gian người dùng (user space) và không gian nhân (kernel space).

## ✨ Tính năng nổi bật

*   **Quản lý bộ nhớ động:** Sử dụng cấu trúc danh sách liên kết (`list_head`) để quản lý các trang bộ nhớ (`struct page`) được cấp phát động. Các trang sẽ được giải phóng hoàn toàn khi thiết bị được mở ở chế độ chỉ ghi (O_WRONLY).
*   **Giao tiếp Device Driver:** Triển khai đầy đủ các thao tác tệp tin cơ bản: `open`, `read`, `write`, `lseek`, `release`.
*   **Hỗ trợ mmap:** Cho phép ánh xạ bộ nhớ từ kernel space trực tiếp vào user space để tối ưu hóa hiệu suất truy cập dữ liệu.
*   **Điều khiển IOCTL:** Cung cấp cơ chế điều khiển nâng cao, cho phép cấu hình số lượng tiến trình tối đa có thể truy cập thiết bị đồng thời.
*   **Giao diện /proc:** Tích hợp giao diện trong hệ thống tệp `/proc/asgn1` để theo dõi trạng thái thiết bị (Major/Minor number, số trang đã cấp phát, kích thước dữ liệu, số tiến trình đang truy cập).

## 📁 Cấu trúc dự án

```
He-Dieu-Hanh-Nhung/
├── asgn1.c         # ⚙️ Mã nguồn chính của Kernel Module
├── mmap_test.c     # 🧪 Chương trình kiểm thử tính năng mmap
├── Makefile        # 🛠️ Tệp cấu hình biên dịch Module
├── README.md       # 📖 Tài liệu hướng dẫn
└── ...
```

## 🛠️ Yêu cầu & Cài đặt

### Yêu cầu
*   Hệ điều hành Linux (Ubuntu, Debian, v.v.)
*   Linux Kernel Headers tương ứng với phiên bản nhân đang chạy.
*   Trình biên dịch GCC và công cụ Make.

### Hướng dẫn cài đặt

<details>
<summary><b>Nhấn để xem chi tiết các bước biên dịch và nạp Module</b></summary>

1.  **Biên dịch Module:**
    ```bash
    make
    ```

2.  **Nạp Module vào nhân:**
    ```bash
    sudo insmod asgn1.ko
    ```

3.  **Kiểm tra thiết bị đã tạo:**
    ```bash
    ls -l /dev/asgn1
    cat /proc/asgn1
    ```

4.  **Gỡ bỏ Module:**
    ```bash
    sudo rmmod asgn1
    ```
</details>

## 🧠 Phân tích kỹ thuật

Dự án triển khai các kỹ thuật lập trình nhân Linux quan trọng:
*   **Cdev API:** Đăng ký và quản lý thiết bị ký tự một cách chuyên nghiệp.
*   **Atomic Operations:** Sử dụng `atomic_t` để quản lý số lượng tiến trình truy cập, đảm bảo an toàn dữ liệu trong môi trường đa nhiệm.
*   **Page Allocation:** Cấp phát bộ nhớ theo đơn vị trang (`alloc_page`) để tận dụng tối đa cơ chế quản lý bộ nhớ của nhân Linux.
*   **Remap PFN Range:** Kỹ thuật cốt lõi trong `mmap` để ánh xạ các trang vật lý vào không gian địa chỉ ảo của tiến trình người dùng.

---

## 🌍 English Summary

### **Virtual RAM Disk Linux Kernel Module**

Developed under the guidance of **Instructor Pham Doan Tinh**, this project implements a **Linux Kernel Module** for a **Virtual RAM Disk**. It functions as a high-performance character device accessible via `/dev/asgn1`.

**Key Technical Features:**
*   **On-demand Memory Allocation:** Automatically allocates memory pages during write operations and manages them via a kernel linked list.
*   **Process Control:** Utilizes **IOCTL** to limit the maximum number of concurrent processes accessing the device.
*   **Memory Efficiency:** Supports **mmap** for direct memory mapping and provides a mechanism to free all allocated pages when opened in write-only mode.
*   **System Integration:** Fully integrated with the `/proc` filesystem for real-time debugging and status monitoring.

This project showcases advanced skills in C programming, Linux kernel internals, and memory management strategies for **Embedded Systems**.

---

## 🤝 Đóng góp

Mọi ý tưởng đóng góp về tối ưu hóa bộ nhớ hoặc thêm tính năng hỗ trợ hệ thống tệp ảo đều được hoan nghênh. Vui lòng gửi Pull Request hoặc tạo Issue.

## 📄 Giấy phép

Dự án này được phát hành dưới Giấy phép GPL.

---

*Tài liệu này được soạn thảo bởi **Manus AI** vào ngày 23 tháng 2 năm 2026.*

*Liên hệ tác giả: [duongdoandanghoc](https://github.com/duongdoandanghoc)*
