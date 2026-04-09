#pragma once

#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Timestamp.h>

#include "../common/request_counter.h"

#include <string>

namespace handlers
{

  inline const std::string SWAGGER_YAML = R"(openapi: 3.0.3
info:
  title: Library Management REST API
  description: REST API for managing users, books and book loans
  version: 1.0.0

servers:
  - url: /

paths:
  /api/v1/auth/register:
    post:
      summary: Register new user
      tags: [Auth]
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/RegisterRequest'
      responses:
        '201':
          description: User created
        '400':
          description: Invalid request
        '409':
          description: User already exists

  /api/v1/auth/login:
    post:
      summary: Login and get JWT token
      tags: [Auth]
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/LoginRequest'
      responses:
        '200':
          description: JWT token returned
        '400':
          description: Invalid request
        '401':
          description: Invalid credentials

  /api/v1/users:
    post:
      summary: Create new user
      tags: [Users]
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/RegisterRequest'
      responses:
        '201':
          description: User created
        '400':
          description: Invalid request
        '409':
          description: User already exists

  /api/v1/users/by-login/{login}:
    get:
      summary: Find user by login
      tags: [Users]
      parameters:
        - in: path
          name: login
          required: true
          schema:
            type: string
      responses:
        '200':
          description: User found
        '404':
          description: User not found

  /api/v1/users/search:
    get:
      summary: Find users by first and last name mask
      tags: [Users]
      parameters:
        - in: query
          name: firstName
          required: false
          schema:
            type: string
        - in: query
          name: lastName
          required: false
          schema:
            type: string
      responses:
        '200':
          description: Search results returned

  /api/v1/books:
    post:
      summary: Add book to library
      tags: [Books]
      security:
        - bearerAuth: []
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/CreateBookRequest'
      responses:
        '201':
          description: Book created
        '400':
          description: Invalid request
        '401':
          description: Unauthorized

  /api/v1/books/search:
    get:
      summary: Find books by title or author
      tags: [Books]
      parameters:
        - in: query
          name: title
          required: false
          schema:
            type: string
        - in: query
          name: author
          required: false
          schema:
            type: string
      responses:
        '200':
          description: Search results returned

  /api/v1/loans:
    post:
      summary: Create a book loan
      tags: [Loans]
      security:
        - bearerAuth: []
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/CreateLoanRequest'
      responses:
        '201':
          description: Loan created
        '400':
          description: Invalid request
        '401':
          description: Unauthorized
        '404':
          description: User or book not found
        '409':
          description: Conflict

  /api/v1/users/{userId}/loans:
    get:
      summary: Get loans of a user
      tags: [Loans]
      parameters:
        - in: path
          name: userId
          required: true
          schema:
            type: integer
      responses:
        '200':
          description: Loans returned
        '404':
          description: User not found

  /api/v1/loans/{loanId}/return:
    patch:
      summary: Return a book
      tags: [Loans]
      security:
        - bearerAuth: []
      parameters:
        - in: path
          name: loanId
          required: true
          schema:
            type: integer
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ReturnLoanRequest'
      responses:
        '200':
          description: Loan updated
        '400':
          description: Invalid request
        '401':
          description: Unauthorized
        '404':
          description: Loan not found
        '409':
          description: Conflict

components:
  securitySchemes:
    bearerAuth:
      type: http
      scheme: bearer
      bearerFormat: JWT

  schemas:
    RegisterRequest:
      type: object
      required: [login, password, firstName, lastName]
      properties:
        login:
          type: string
          example: reader1
        password:
          type: string
          example: secret123
        firstName:
          type: string
          example: Ivan
        lastName:
          type: string
          example: Petrov

    LoginRequest:
      type: object
      required: [login, password]
      properties:
        login:
          type: string
          example: reader1
        password:
          type: string
          example: secret123

    CreateBookRequest:
      type: object
      required: [title, author]
      properties:
        title:
          type: string
          example: War and Peace
        author:
          type: string
          example: Leo Tolstoy
        year:
          type: integer
          example: 1869

    CreateLoanRequest:
      type: object
      required: [userId, bookId, issuedAt]
      properties:
        userId:
          type: integer
          example: 1
        bookId:
          type: integer
          example: 1
        issuedAt:
          type: string
          example: 2026-03-28

    ReturnLoanRequest:
      type: object
      required: [returnedAt]
      properties:
        returnedAt:
          type: string
          example: 2026-04-05

    ErrorResponse:
      type: object
      properties:
        error:
          type: string
          example: user not found
)";

  class SwaggerHandler : public Poco::Net::HTTPRequestHandler
  {
  public:
    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override
    {
      Poco::Timestamp start;
      if (g_httpRequests)
        g_httpRequests->inc();

      response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
      response.setContentType("application/x-yaml");
      response.setContentLength(static_cast<std::streamsize>(SWAGGER_YAML.size()));
      std::ostream &ostr = response.send();
      ostr << SWAGGER_YAML;

      Poco::Timespan elapsed = Poco::Timestamp() - start;
      double seconds = static_cast<double>(elapsed.totalMicroseconds()) / 1000000.0;
      if (g_httpDuration)
        g_httpDuration->observe(seconds);

      auto &logger = Poco::Logger::get("Server");
      logger.information("200 GET /swagger.yaml from %s, %.2f ms",
                         request.clientAddress().toString(), seconds * 1000.0);
    }
  };

}